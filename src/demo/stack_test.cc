#include "gtest/gtest.h"
#include "stack_c.h"
#include <string>

using namespace std;
namespace latte {

    typedef uint64_t Key;

    struct Comparator {
        int operator()(const Key& a, const Key& b) const {
            if (a < b) {
                return -1;
            } else if (a > b) {
                return +1;
            } else {
                return 0;
            }
        }
    };

    TEST(StackTest, Empty) {
        try { 
            Stack<int>         intStack;  // int 类型的栈 
            Stack<string> stringStack;    // string 类型的栈 
    
            // 操作 int 类型的栈 
            intStack.push(7); 
            cout << intStack.top() <<endl; 
    
            // 操作 string 类型的栈 
            stringStack.push("hello"); 
            cout << stringStack.top() << std::endl; 
            stringStack.pop(); 
            stringStack.pop(); 
        } 
        catch (exception const& ex) { 
            cerr << "Exception: " << ex.what() <<endl;
        } 
    }
}