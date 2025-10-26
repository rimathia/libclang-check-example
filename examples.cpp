#include <Eigen/Dense>
#include <iostream>

using namespace Eigen;

void example1_repeated_evaluation() {
    // Deduces expression template type - multiplication runs repeatedly
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = A * B;  // Deduces Eigen::Product<...>, not MatrixXd

    // Each access to C recomputes A*B
    std::cout << "C(0,0): " << C(0, 0) << std::endl;
    std::cout << "C(1,1): " << C(1, 1) << std::endl;
}

void example1b_const_auto() {
    // Deduces expression template type - const doesn't change deduced type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto C = A * B;  // Deduces const Eigen::Product<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example1c_const_auto_ref() {
    // Deduces expression template type - reference doesn't change deduced type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto& C = A * B;  // Deduces const Eigen::Product<...> &

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example2_stale_references() {
    // Deduces expression template type - stores references to operands
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = A * B;  // Deduces Eigen::Product<...> - stores references to A and B

    std::cout << "Before: " << C(0, 0) << std::endl;
    A(0, 0) = 999.0;  // Modifying A changes C's result!
    std::cout << "After: " << C(0, 0) << std::endl;
}

void example3_dangling_reference() {
    // Deduces expression template type - temporary deleted while referenced
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = ((A + B).eval()).transpose();  // Deduces Eigen::Transpose<...>

    // Accessing C is undefined behavior - segfault risk
    // std::cout << C(0, 0) << std::endl;
}

void example3b_const_auto_dangling() {
    // Deduces expression template type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto C = ((A + B).eval()).transpose();  // Deduces const Eigen::Transpose<...>

    // std::cout << C(0, 0) << std::endl;
}

void example3c_const_auto_ref_dangling() {
    // Deduces expression template type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto& C = ((A + B).eval()).transpose();  // Deduces const Eigen::Transpose<...> &

    // std::cout << C(0, 0) << std::endl;
}

void example4_correct_with_eval() {
    // Deduces plain matrix type - eval() materializes the result
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = (A * B).eval();  // Deduces Eigen::Matrix<double, -1, -1>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example4b_const_auto_eval() {
    // Deduces plain matrix type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto C = (A * B).eval();  // Deduces const Eigen::Matrix<double, -1, -1>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example4c_const_auto_ref_eval() {
    // Deduces plain matrix type (but const& to temporary has lifetime issues)
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto& C = (A * B).eval();  // Deduces const Eigen::Matrix<double, -1, -1> &

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example5_correct_explicit_type() {
    // Explicit type declaration (recommended pattern) - no auto
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    MatrixXd C = A * B;  // Explicit MatrixXd - forces evaluation

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example6_auto_with_plain_matrix() {
    // Deduces plain matrix type - copying from plain matrix
    MatrixXd A = MatrixXd::Random(3, 3);

    auto B = A;  // Deduces Eigen::Matrix<double, -1, -1>

    std::cout << "B(0,0): " << B(0, 0) << std::endl;
}

void example6b_const_auto_plain() {
    // Deduces plain matrix type
    MatrixXd A = MatrixXd::Random(3, 3);

    const auto B = A;  // Deduces const Eigen::Matrix<double, -1, -1>

    std::cout << "B(0,0): " << B(0, 0) << std::endl;
}

void example6c_const_auto_ref_plain() {
    // Deduces plain matrix type - reference to existing matrix
    MatrixXd A = MatrixXd::Random(3, 3);

    const auto& B = A;  // Deduces const Eigen::Matrix<double, -1, -1> &

    std::cout << "B(0,0): " << B(0, 0) << std::endl;
}

void example7_complex_expression() {
    // Deduces expression template type - complex nested expression
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = A * B + D.transpose();  // Deduces Eigen::CwiseBinaryOp<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example7b_const_auto_complex() {
    // Deduces expression template type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    const auto C = A * B + D.transpose();  // Deduces const Eigen::CwiseBinaryOp<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example7c_const_auto_ref_complex() {
    // Deduces expression template type
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    const auto& C = A * B + D.transpose();  // Deduces const Eigen::CwiseBinaryOp<...> &

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example8_vector_normalized() {
    // Deduces expression template type
    MatrixXd A = MatrixXd::Random(3, 3);
    VectorXd v = VectorXd::Random(3);
    VectorXd u = VectorXd::Random(3);

    auto C = u + (A * v).normalized();  // Deduces Eigen::CwiseBinaryOp<...>

    std::cout << "C(0): " << C(0) << std::endl;
}

void example9_decltype_auto() {
    // Deduces expression template type - decltype(auto) behaves like auto here
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    decltype(auto) C = A * B;  // Deduces Eigen::Product<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example10_auto_ref() {
    // Deduces expression template type - auto& behaves same as const auto&
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto& C = A * B;  // Deduces const Eigen::Product<...> &

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

// Helper function that returns a VectorXd
VectorXd compute_result(const MatrixXd& A, const VectorXd& v) {
    return A * v;  // Returns a plain VectorXd (expression is evaluated)
}

void example11_function_return_value() {
    // Deduces plain vector type - function returns VectorXd by value
    MatrixXd A = MatrixXd::Random(3, 3);
    VectorXd v = VectorXd::Random(3);

    auto result = compute_result(A, v);  // Deduces Eigen::Matrix<double, -1, 1>

    std::cout << "result(0): " << result(0) << std::endl;
}

// ============================================================================
// Multi-line expression examples
// ============================================================================

void example_multiline1_expression_template() {
    // Deduces expression template type - multi-line expression
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = A * B +
             D.transpose();  // Deduces Eigen::CwiseBinaryOp<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example_multiline2_safe_eval() {
    // Deduces plain matrix type - multi-line with eval()
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = (A * B +
              D.transpose()).eval();  // Deduces Eigen::Matrix<double, -1, -1>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example_multiline3_complex_expression() {
    // Deduces expression template type - complex multi-line expression
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);
    MatrixXd E = MatrixXd::Random(3, 3);

    auto C = A * B +
             D.transpose() *
             E;  // Deduces Eigen::CwiseBinaryOp<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example_multiline4_parenthesized() {
    // Deduces expression template type - parentheses don't materialize
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = (
        A * B +
        D.transpose()
    );  // Deduces Eigen::CwiseBinaryOp<...>

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

// ============================================================================
// Non-Eigen examples: Same syntax with doubles to verify check doesn't fire
// ============================================================================

void example12_auto_with_double() {
    // Deduces double - no Eigen involved
    double a = 3.14;
    double b = 2.71;

    auto c = a * b;  // Deduces double

    std::cout << "c: " << c << std::endl;
}

void example13_auto_with_double_copy() {
    // Deduces double - no Eigen involved
    double a = 42.0;

    auto b = a;  // Deduces double

    std::cout << "b: " << b << std::endl;
}

double compute_double_result(double a, double b) {
    return a * b;
}

void example14_auto_with_double_function() {
    // Deduces double - no Eigen involved
    double a = 3.14;
    double b = 2.71;

    auto result = compute_double_result(a, b);  // Deduces double

    std::cout << "result: " << result << std::endl;
}

void example15_decltype_auto_with_double() {
    // Deduces double - no Eigen involved
    double a = 3.14;
    double b = 2.71;

    decltype(auto) c = a * b;  // Deduces double

    std::cout << "c: " << c << std::endl;
}

void example16_auto_ref_with_double() {
    // Deduces double reference - no Eigen involved (but lifetime issue with temporary)
    double a = 3.14;
    double b = 2.71;

    const auto& c = a * b;  // Deduces const double &

    std::cout << "c: " << c << std::endl;
}

int main() {
    std::cout << "=== Example 1: Repeated Evaluation ===" << std::endl;
    example1_repeated_evaluation();

    std::cout << "\n=== Example 1b: const auto ===" << std::endl;
    example1b_const_auto();

    std::cout << "\n=== Example 1c: const auto& ===" << std::endl;
    example1c_const_auto_ref();

    std::cout << "\n=== Example 2: Stale References ===" << std::endl;
    example2_stale_references();

    std::cout << "\n=== Example 4: Correct with eval() ===" << std::endl;
    example4_correct_with_eval();

    std::cout << "\n=== Example 4b: const auto with eval() ===" << std::endl;
    example4b_const_auto_eval();

    std::cout << "\n=== Example 4c: const auto& with eval() ===" << std::endl;
    example4c_const_auto_ref_eval();

    std::cout << "\n=== Example 5: Correct Explicit Type ===" << std::endl;
    example5_correct_explicit_type();

    std::cout << "\n=== Example 6: Auto with Plain Matrix ===" << std::endl;
    example6_auto_with_plain_matrix();

    std::cout << "\n=== Example 6b: const auto with Plain Matrix ===" << std::endl;
    example6b_const_auto_plain();

    std::cout << "\n=== Example 6c: const auto& with Plain Matrix ===" << std::endl;
    example6c_const_auto_ref_plain();

    std::cout << "\n=== Example 7: Complex Expression ===" << std::endl;
    example7_complex_expression();

    std::cout << "\n=== Example 7b: const auto Complex ===" << std::endl;
    example7b_const_auto_complex();

    std::cout << "\n=== Example 7c: const auto& Complex ===" << std::endl;
    example7c_const_auto_ref_complex();

    std::cout << "\n=== Example 8: Vector Normalized ===" << std::endl;
    example8_vector_normalized();

    std::cout << "\n=== Example 9: decltype(auto) ===" << std::endl;
    example9_decltype_auto();

    std::cout << "\n=== Example 10: auto& ===" << std::endl;
    example10_auto_ref();

    std::cout << "\n=== Example 11: Function Return Value ===" << std::endl;
    example11_function_return_value();

    std::cout << "\n=== Multi-line 1: Expression Template ===" << std::endl;
    example_multiline1_expression_template();

    std::cout << "\n=== Multi-line 2: Safe with eval() ===" << std::endl;
    example_multiline2_safe_eval();

    std::cout << "\n=== Multi-line 3: Complex Expression ===" << std::endl;
    example_multiline3_complex_expression();

    std::cout << "\n=== Multi-line 4: Parenthesized ===" << std::endl;
    example_multiline4_parenthesized();

    std::cout << "\n=== Example 12: Auto with Double ===" << std::endl;
    example12_auto_with_double();

    std::cout << "\n=== Example 13: Auto with Double Copy ===" << std::endl;
    example13_auto_with_double_copy();

    std::cout << "\n=== Example 14: Auto with Double Function ===" << std::endl;
    example14_auto_with_double_function();

    std::cout << "\n=== Example 15: decltype(auto) with Double ===" << std::endl;
    example15_decltype_auto_with_double();

    std::cout << "\n=== Example 16: auto& with Double ===" << std::endl;
    example16_auto_ref_with_double();

    return 0;
}
