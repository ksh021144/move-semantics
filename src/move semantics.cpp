#include <iostream>
#include <cstring>
#include <algorithm>
#include <utility>
#include <vector>
#include <chrono>
using namespace std;

#define CONSOLE_PRINT_USE // 필요에 따라 이 선언을 제거하고 실행

class A {
private:
    int* mData;
    int mLength;

public:

    // Default constructor
    A() : mData(nullptr), mLength(0) { }

    A(int pLength)
        :mLength(pLength), mData(new int[pLength]) {
#ifdef CONSOLE_PRINT_USE
        cout << "A(size_t). length = " << mLength << ".\n";
#endif
        for (int i = 0; i < pLength; ++i) {
            mData[i] = i;
        }
    }

    ~A() {
#ifdef CONSOLE_PRINT_USE
        cout << "~A(). length = " << mLength << ".";
#endif
        if (mData != nullptr) {
#ifdef CONSOLE_PRINT_USE
            cout << " Deleting resource.";
#endif
            delete[] mData;
            mData = nullptr;
        }
#ifdef CONSOLE_PRINT_USE
        cout << "\n";
#endif
    }

    void swap(A& other) {
        A temp(other);
        other = *this;
        *this = temp;
    }

    void move_swap(A& other) {
        A temp = std::move(other);
        other = std::move(*this);
        *this = std::move(temp);
    }

    // std::move를 사용할 때와 동일함
    void move_swap_same(A& other) {
        A temp = static_cast<A&&>(other);
        other = static_cast<A&&>(*this);
        *this = static_cast<A&&>(temp);
    }


    // 복사 생성자
    A(const A& other) : mLength(other.mLength), mData(new int[other.mLength]) {
#ifdef CONSOLE_PRINT_USE
        cout << "A(const A&). length = " << other.mLength << ". Copying resource." << "\n";
#endif
        copy(other.mData, other.mData + mLength, mData);
    }

    // 복사 할당 연산자
    A& operator=(const A& other) {
#ifdef CONSOLE_PRINT_USE
        cout << "operator=(A&). length = " << other.mLength << ". Copying resource." << "\n";
#endif
        if (this != &other) {
            delete[] mData;
            mLength = other.mLength;
            mData = new int[mLength];
            copy(other.mData, other.mData + mLength, mData);
        }
        return *this;
    }

    int* get_mData() const {
        return mData;
    }

    // 이동 생성자
    A(A&& other) noexcept {
#ifdef CONSOLE_PRINT_USE
        cout << "A(A&&). length = " << other.mLength << ". Moving resource.\n";
#endif

        mData = other.mData; // 포인터를 복사합니다.
        mLength = other.mLength;

        // 같은 주소에 대해 메모리 해제를 여러 번 하는 것을 방지하기 위해 nullptr로 설정합니다.
        other.mData = nullptr;
        other.mLength = 0;
    }

    // 이동 할당 연산자
    A& operator=(A&& other) noexcept {
#ifdef CONSOLE_PRINT_USE
        cout << "operator=(A&&). length = " << other.mLength << "." << "\n";
#endif

        if (this != &other) { // 자기 자신을 대입하는 행위를 방지 ex) aObj = aObj;
#ifdef CONSOLE_PRINT_USE
            cout << "Address: " << mData << "\n";
#endif
            delete[] mData; // 할당될 개체의 메모리를 해제합니다.

            mData = other.mData; // 포인터를 복사합니다.
            mLength = other.mLength;

            // 같은 주소에 대해 메모리 해제를 여러 번 하는 것을 방지하기 위해 nullptr로 설정합니다.
            other.mData = nullptr;
            other.mLength = 0;
        }

        return *this; // 현재 개체에 대한 참조를 반환합니다.
    }
};


int main() {

    A a(2), b(5);
    cout << a.get_mData() << " " << b.get_mData() << "\n";
    a.swap(b); // copy swap
    cout << a.get_mData() << " " << b.get_mData() << "\n";

    cout << "------------------\n";
    cout << a.get_mData() << " " << b.get_mData() << "\n";
    a.move_swap(b); // move swap
    cout << a.get_mData() << " " << b.get_mData() << "\n";

#ifdef _WIN64
    // 아래 코드에 의해 할당되는 메모리가 4GB를 초과하므로 x64 환경에서 실행해야 정상 작동합니다.
    // Visual Studio 사용자의 경우 구성 관리자를 [x64]로 변경하세요.
    cout << "------------------\n";
    vector<A> v;
    std::chrono::system_clock::time_point startTime = chrono::system_clock::now();
    for (int i = 0; i < 1100000; ++i) {
        v.push_back(A(1000));
    }
    chrono::duration<double> sec = chrono::system_clock::now() - startTime;
    cout << sec.count() << " sec\n";
#endif

#ifdef CONSOLE_PRINT_USE
    cout << "------------------\n";
    cout << "destructor\n";
#endif
    return 0;
}