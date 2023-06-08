// Coder
// MIT License. Copyright 2023 Mirko Palmer (derbroti)
////////

template <typename = void, typename = void>
void REQUIRES() {}

template <typename T, typename U, typename... Args>
void REQUIRES(T got, U want, Args... args) {
    REQUIRE(got == want);
    REQUIRES(args...);
}

