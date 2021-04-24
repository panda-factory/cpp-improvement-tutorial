//
// Created by admin on 2021-04-24.
//

#ifndef TEST_SHARED_H
#define TEST_SHARED_H

#if defined(USE_STATIC)
#define EXPORT_DLL
#else // USE_STATIC
#if defined(MAKE_DLL)
#define EXPORT_DLL     __declspec(dllexport)
#else // MAKE_DLL
#define EXPORT_DLL     __declspec(dllimport)
#endif // MAKE_DLL
#endif// USE_STATIC

class AllStatic {
    AllStatic() = delete;
};

class EXPORT_DLL Shared : public AllStatic {
public:
    static void Test();
};

#endif //TEST_SHARED_H
