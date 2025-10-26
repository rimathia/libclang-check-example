#include <Eigen/Dense>
#include <iostream>

using namespace Eigen;

void example1_repeated_evaluation() {
    // UNSAFE: auto captures expression template, multiplication runs repeatedly
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = A * B;  // UNSAFE - deduces expression template type, not MatrixXd

    // Each access to C recomputes A*B
    std::cout << "C(0,0): " << C(0, 0) << std::endl;
    std::cout << "C(1,1): " << C(1, 1) << std::endl;
}

void example2_stale_references() {
    // UNSAFE: Changing A affects what C represents
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = A * B;  // UNSAFE - stores references to A and B, not the result

    std::cout << "Before: " << C(0, 0) << std::endl;
    A(0, 0) = 999.0;  // Modifying A changes C's result!
    std::cout << "After: " << C(0, 0) << std::endl;
}

void example3_dangling_reference() {
    // UNSAFE: Temporary deleted while referenced
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = ((A + B).eval()).transpose();  // UNSAFE - dangling reference to temporary

    // Accessing C is undefined behavior - segfault risk
    // std::cout << C(0, 0) << std::endl;
}

void example4_correct_with_eval() {
    // SAFE: Using eval() on the complete expression materializes the result
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    auto C = (A * B).eval();  // SAFE - eval() returns plain matrix, but uses auto

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example5_correct_explicit_type() {
    // SAFE: Explicit type declaration (recommended pattern)
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    MatrixXd C = A * B;  // SAFE - explicit type forces evaluation

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example6_auto_with_plain_matrix() {
    // SAFE: auto with already-plain matrix (technically safe, potential false positive)
    MatrixXd A = MatrixXd::Random(3, 3);

    auto B = A;  // SAFE - copying plain MatrixXd, no expression template involved

    std::cout << "B(0,0): " << B(0, 0) << std::endl;
}

void example7_complex_expression() {
    // UNSAFE: Complex expression template with multiple operations
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = A * B + D.transpose();  // UNSAFE - complex expression template

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example8_vector_normalized() {
    // UNSAFE: Automatic evaluation with potential dangling reference
    MatrixXd A = MatrixXd::Random(3, 3);
    VectorXd v = VectorXd::Random(3);
    VectorXd u = VectorXd::Random(3);

    auto C = u + (A * v).normalized();  // UNSAFE - normalized() creates temporary

    std::cout << "C(0): " << C(0) << std::endl;
}

void example9_decltype_auto() {
    // UNSAFE: decltype(auto) has same issues as auto
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    decltype(auto) C = A * B;  // UNSAFE - deduces expression template like auto

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example10_auto_ref() {
    // UNSAFE: auto& or const auto& still captures expression template
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);

    const auto& C = A * B;  // UNSAFE - reference to expression template

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

// Helper function that returns a VectorXd
VectorXd compute_result(const MatrixXd& A, const VectorXd& v) {
    return A * v;  // Returns a plain VectorXd (expression is evaluated)
}

void example11_function_return_value() {
    // SAFE: Function returns VectorXd by value, auto deduces plain type
    MatrixXd A = MatrixXd::Random(3, 3);
    VectorXd v = VectorXd::Random(3);

    auto result = compute_result(A, v);  // SAFE - function returns plain VectorXd

    std::cout << "result(0): " << result(0) << std::endl;
}

// ============================================================================
// Multi-line expression examples
// ============================================================================

void example_multiline1_expression_template() {
    // UNSAFE: Multi-line expression template
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = A * B +
             D.transpose();  // UNSAFE - expression template across lines

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example_multiline2_safe_eval() {
    // SAFE: Multi-line with eval()
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = (A * B +
              D.transpose()).eval();  // SAFE - eval() returns plain matrix

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example_multiline3_complex_expression() {
    // UNSAFE: Complex multi-line expression
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);
    MatrixXd E = MatrixXd::Random(3, 3);

    auto C = A * B +
             D.transpose() *
             E;  // UNSAFE - complex expression template

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

void example_multiline4_parenthesized() {
    // UNSAFE: Expression in parentheses across lines
    MatrixXd A = MatrixXd::Random(3, 3);
    MatrixXd B = MatrixXd::Random(3, 3);
    MatrixXd D = MatrixXd::Random(3, 3);

    auto C = (
        A * B +
        D.transpose()
    );  // UNSAFE - parentheses don't materialize the result

    std::cout << "C(0,0): " << C(0, 0) << std::endl;
}

// ============================================================================
// Non-Eigen examples: Same syntax with doubles to verify check doesn't fire
// ============================================================================

void example12_auto_with_double() {
    // SAFE: auto with double (no Eigen involved)
    double a = 3.14;
    double b = 2.71;

    auto c = a * b;  // SAFE - auto deduces double, not expression template

    std::cout << "c: " << c << std::endl;
}

void example13_auto_with_double_copy() {
    // SAFE: auto copying a double
    double a = 42.0;

    auto b = a;  // SAFE - auto deduces double

    std::cout << "b: " << b << std::endl;
}

double compute_double_result(double a, double b) {
    return a * b;
}

void example14_auto_with_double_function() {
    // SAFE: auto with function returning double
    double a = 3.14;
    double b = 2.71;

    auto result = compute_double_result(a, b);  // SAFE - auto deduces double

    std::cout << "result: " << result << std::endl;
}

void example15_decltype_auto_with_double() {
    // SAFE: decltype(auto) with double (no Eigen involved)
    double a = 3.14;
    double b = 2.71;

    decltype(auto) c = a * b;  // SAFE - deduces double

    std::cout << "c: " << c << std::endl;
}

void example16_auto_ref_with_double() {
    // SAFE: const auto& with double (no Eigen involved)
    double a = 3.14;
    double b = 2.71;

    const auto& c = a * b;  // SAFE - reference to double (though unusual)

    std::cout << "c: " << c << std::endl;
}

int main() {
    std::cout << "=== Example 1: Repeated Evaluation ===" << std::endl;
    example1_repeated_evaluation();

    std::cout << "\n=== Example 2: Stale References ===" << std::endl;
    example2_stale_references();

    std::cout << "\n=== Example 4: Correct with eval() ===" << std::endl;
    example4_correct_with_eval();

    std::cout << "\n=== Example 5: Correct Explicit Type ===" << std::endl;
    example5_correct_explicit_type();

    std::cout << "\n=== Example 6: Auto with Plain Matrix ===" << std::endl;
    example6_auto_with_plain_matrix();

    std::cout << "\n=== Example 7: Complex Expression ===" << std::endl;
    example7_complex_expression();

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
