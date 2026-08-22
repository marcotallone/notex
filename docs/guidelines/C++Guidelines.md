# C++ Guidelines

This document describes the `C++` coding guidelines, formatting preferences and documentation indications.

## C++ Formatting Directives

- **Base style**: Google
- **Indentation**: 4 spaces. No tabs.
- **Line Length**: Max 80 characters.
- **Access Modifiers**: Indent access modifiers by -4 spaces.
- **Brace Style**: Egiptian style (opening brace on the same line).

## C++ Documentation Directives

- Use **Doxygen** style comments for all public members.
- Use Javadoc style comments `/** ... */` for long documentation.
- Use triple slashes `///` only for single line documentation.
- Use Javadoc `@` symbol: document parameters using `@param`, return values with `@return`, exceptions with `@throws`, etc...
- Use `@p` symbol to correctly mark parameters in classes and methods
  documentation
- For classes, document both the class and its constructor as well as other
  members, but in a specific
  way according to the table below:

| Element                           | Placement                             | Purpose                                                          |
| --------------------------------- | ------------------------------------- | ---------------------------------------------------------------- |
| **Class documentation**           | Before class definition               | Describes overall purpose, usage, and high-level behavior        |
| **Constructor documentation**     | Before each constructor               | Documents parameters, initialization behavior, and exceptions    |
| **Method documentation**          | Before each method                    | Explains what the method does, its parameters, and return values |
| **Member variable documentation** | Inline (after the variable) or before | Explains what each member stores                                 |

- To document the members of a file, struct, union, class, or enum, place the documentation block after the member instead of before. For this purpose, use the `///<` syntax. E.g., `int var; ///< Brief description after the member`.

### Example

Small trivial example.

---

**Before Documentation\***:

```cpp
// Method to cast an Image to OpenCV matrix
inline cv::Mat opencv(const Image& img, int rows, int cols) {
    // Error if the image size does not match the expected one
    if (img.rows() != rows * cols) {
        throw std::invalid_argument(
            "Dimension mismatch: Image rows must equal rows * cols.");
    }

    // WARNING: remember to keep the source Image alive in scope
    //          to avoid breaking reference
    return cv::Mat(rows, cols, CV_32FC3, const_cast<float*>(img.data()));
}

```

---

**After Documentation**:

```cpp
/**
 * @brief Zero-copy maps an Eigen Image back to an OpenCV cv::Mat
 *        representation.
 *
 * Creates an OpenCV matrix header of shape (@p rows, @p cols) that references the underlying Eigen data.
 * No copy is performed; the returned matrix shares memory with the source.
 *
 * @param img Input Eigen Image matrix (rows*cols, 3).
 * @param rows Image height in pixels.
 * @param cols Image width in pixels.
 * @return OpenCV cv::Mat (rows, cols, CV_32FC3) with shared ownership
 *         of Eigen data.
 *
 * @throws std::invalid_argument if img.rows() != rows * cols.
 *
 * @warning The returned cv::Mat references the Eigen memory buffer. Keep the
 *          source Image alive while using the returned matrix.
 */
inline cv::Mat opencv(const Image& img, int rows, int cols) {
    // Error if the image size does not match the expected one
    if (img.rows() != rows * cols) {
        throw std::invalid_argument(
            "Dimension mismatch: Image rows must equal rows * cols.");
    }

    // WARNING: remember to keep the source Image alive in scope
    //          to avoid breaking reference
    return cv::Mat(rows, cols, CV_32FC3, const_cast<float*>(img.data()));
}
```
