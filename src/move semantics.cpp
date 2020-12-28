#include <iostream>
#include <cstring>
#include <algorithm>
#include <utility>
#include <vector>
#include <chrono>
using namespace std;

#define CONSOLE_PRINT_USE // �ʿ信 ���� �� ������ �����ϰ� ����

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

    // std::move�� ����� ���� ������
    void move_swap_same(A& other) {
        A temp = static_cast<A&&>(other);
        other = static_cast<A&&>(*this);
        *this = static_cast<A&&>(temp);
    }


    // ���� ������
    A(const A& other) : mLength(other.mLength), mData(new int[other.mLength]) {
#ifdef CONSOLE_PRINT_USE
        cout << "A(const A&). length = " << other.mLength << ". Copying resource." << "\n";
#endif
        copy(other.mData, other.mData + mLength, mData);
    }

    // ���� �Ҵ� ������
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

    // �̵� ������
    A(A&& other) noexcept {
#ifdef CONSOLE_PRINT_USE
        cout << "A(A&&). length = " << other.mLength << ". Moving resource.\n";
#endif

        mData = other.mData; // �����͸� �����մϴ�.
        mLength = other.mLength;

        // ���� �ּҿ� ���� �޸� ������ ���� �� �ϴ� ���� �����ϱ� ���� nullptr�� �����մϴ�.
        other.mData = nullptr;
        other.mLength = 0;
    }

    // �̵� �Ҵ� ������
    A& operator=(A&& other) noexcept {
#ifdef CONSOLE_PRINT_USE
        cout << "operator=(A&&). length = " << other.mLength << "." << "\n";
#endif

        if (this != &other) { // �ڱ� �ڽ��� �����ϴ� ������ ���� ex) aObj = aObj;
#ifdef CONSOLE_PRINT_USE
            cout << "Address: " << mData << "\n";
#endif
            delete[] mData; // �Ҵ�� ��ü�� �޸𸮸� �����մϴ�.

            mData = other.mData; // �����͸� �����մϴ�.
            mLength = other.mLength;

            // ���� �ּҿ� ���� �޸� ������ ���� �� �ϴ� ���� �����ϱ� ���� nullptr�� �����մϴ�.
            other.mData = nullptr;
            other.mLength = 0;
        }

        return *this; // ���� ��ü�� ���� ������ ��ȯ�մϴ�.
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
    // �Ʒ� �ڵ忡 ���� �Ҵ�Ǵ� �޸𸮰� 4GB�� �ʰ��ϹǷ� x64 ȯ�濡�� �����ؾ� ���� �۵��մϴ�.
    // Visual Studio ������� ��� ���� �����ڸ� [x64]�� �����ϼ���.
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