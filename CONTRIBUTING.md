# Contributing to AthiVegam

Thank you for your interest in contributing to AthiVegam! This document provides guidelines and workflows for contributing to the project.

## Code of Conduct

- Be respectful and constructive in all interactions
- Focus on what is best for the project and community
- Show empathy towards other community members

## Getting Started

1. Read [BUILD.md](BUILD.md) to set up your development environment
2. Read [TESTING.md](TESTING.md) to understand testing requirements
3. Check the [memory bank](Documentation/) for architecture and standards
4. Review [PHASE0_PLAN.md](PHASE0_PLAN.md) for current development priorities

## Development Workflow

### 1. Create a Branch

```bash
# Feature branch
git checkout -b feature/module-name

# Bug fix branch
git checkout -b fix/issue-description
```

### 2. Make Changes

- Follow coding standards in [development-standards.md](Documentation/development-standards.md)
- Write tests for new functionality
- Update documentation as needed
- Keep commits focused and atomic

### 3. Test Your Changes

```bash
# Build and run tests
cmake --preset windows-debug
cmake --build build/debug
ctest --preset windows-debug --output-on-failure
```

### 4. Commit Changes

Follow commit message format:

```
[Module] Brief description (50 chars or less)

Detailed explanation of changes (wrap at 72 chars):
- What was changed
- Why it was changed
- How it was implemented

Fixes #123 (if applicable)
```

Examples:
```
[Core] Implement FrameArena allocator

- Add bump allocation with per-frame reset
- Include alignment support and bounds checking
- Add comprehensive unit tests

[ECS] Fix archetype migration bug

- Preserve component data during migration
- Add test case for edge case with empty chunks
- Fixes #45
```

### 5. Push and Create Pull Request

```bash
git push origin feature/module-name
```

Then create a Pull Request on GitHub with:
- Clear title describing the change
- Description of what, why, and how
- Link to related issues
- Screenshots/examples if applicable

## Coding Standards

### C++ Style

- **Naming**: See [development-standards.md](Documentation/development-standards.md)
  - Types: `PascalCase`
  - Functions: `snake_case()`
  - Variables: `snake_case`
  - Members: `snake_case_`
  - Constants: `PascalCase`

- **Formatting**:
  - 4 spaces (no tabs)
  - Allman braces (braces on new line)
  - 120 character line limit
  - `#pragma once` for header guards

- **Documentation**:
  - Doxygen comments for public APIs
  - Explain *why*, not *what*
  - Include examples for complex APIs

### Example

```cpp
/// @brief Frame-scoped bump allocator
/// @details Allocates memory with bump pointer, resets each frame.
///          Fast allocation, no individual deallocation.
/// @example
/// @code
/// FrameArena arena(1 * MiB);
/// void* ptr = arena.allocate(64, 16);
/// // ... use memory ...
/// arena.reset(); // Free all at once
/// @endcode
class FrameArena {
public:
    /// @brief Construct arena with capacity
    /// @param capacity Maximum bytes to allocate
    explicit FrameArena(usize capacity);
    
    /// @brief Allocate aligned memory
    /// @param size Bytes to allocate
    /// @param alignment Alignment requirement (power of 2)
    /// @return Pointer to allocated memory, or nullptr if out of space
    void* allocate(usize size, usize alignment = alignof(std::max_align_t));
    
    /// @brief Reset allocator (frees all allocations)
    void reset();
    
private:
    byte* buffer_;
    usize capacity_;
    usize offset_;
};
```

## Testing Requirements

### Unit Tests

- **Required**: All new functionality must have unit tests
- **Coverage**: Aim for > 70% code coverage
- **Framework**: GoogleTest for C++, xUnit for C#

```cpp
TEST(FrameArena, BasicAllocation) {
    FrameArena arena(1024);
    
    void* ptr = arena.allocate(64);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(arena.used(), 64);
}

TEST(FrameArena, AlignedAllocation) {
    FrameArena arena(1024);
    
    void* ptr = arena.allocate(64, 64);
    EXPECT_NE(ptr, nullptr);
    EXPECT_TRUE(is_aligned(ptr, 64));
}
```

### Integration Tests

- Test module interactions
- Located in `tests/integration/`
- Run as part of CI

### Performance Tests

- Benchmark critical paths
- Located in `tests/performance/`
- Document performance expectations

## Pull Request Process

### Before Submitting

- [ ] Code follows project style guidelines
- [ ] All tests pass locally
- [ ] New tests added for new functionality
- [ ] Documentation updated
- [ ] Commit messages follow format
- [ ] No merge conflicts with main branch

### Review Process

1. **Automated Checks**: CI must pass (build, tests, linting)
2. **Code Review**: At least one approval required
3. **Testing**: Reviewer verifies tests are adequate
4. **Documentation**: Reviewer checks documentation completeness

### After Approval

- Squash commits if requested
- Rebase on latest main if needed
- Maintainer will merge when ready

## Module-Specific Guidelines

### Core Module

- No dependencies on other engine modules
- Platform-specific code isolated in Platform/
- Comprehensive error handling
- Extensive unit tests

### ECS Module

- Performance-critical: profile changes
- Maintain cache-friendly data layouts
- Test migration correctness
- Document query performance characteristics

### Rendering Module

- Test on both D3D12 and Vulkan
- Validate with graphics debugger (RenderDoc, PIX)
- Document shader requirements
- Include visual tests when possible

## Documentation

### Code Documentation

- Public APIs: Full Doxygen comments
- Private functions: Brief comments explaining purpose
- Complex algorithms: Detailed explanation with references

### User Documentation

- Update relevant guides in `docs/guides/`
- Add examples to `examples/` if applicable
- Update README.md if user-facing changes

### Architecture Documentation

- Update memory bank files for architectural changes
- Document design decisions and rationale
- Include diagrams for complex systems

## Issue Reporting

### Bug Reports

Include:
- Clear description of the bug
- Steps to reproduce
- Expected vs actual behavior
- Environment (OS, compiler, build configuration)
- Relevant logs or error messages

### Feature Requests

Include:
- Clear description of the feature
- Use cases and motivation
- Proposed API or design (if applicable)
- Alternatives considered

## Communication

- **GitHub Issues**: Bug reports, feature requests
- **Pull Requests**: Code review and discussion
- **Email**: srinathupadhyayula@gmail.com for private matters

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (see [LICENSE](LICENSE)).

## Questions?

- Check existing issues and pull requests
- Read the documentation in `Documentation/`
- Ask in GitHub Discussions (when enabled)
- Email: srinathupadhyayula@gmail.com

Thank you for contributing to AthiVegam!
