
#pragma once 
#include <string>
#include <iostream>

namespace latte
{
    class L {
        public:
            char* name;
    };
    namespace A
    {
        class L: public latte::L {
            public:
            L(char* a) {
                name=a;
            }
             
            void hello() {
                std::cout << "A::L " << name << std::endl;
            }

        };
    } // namespace A

    namespace B
    {
        class L: public latte::L {
            public:
            L(char* b) {
                name = b;
            }
         
            void hello() {
                std::cout << "B::L " << name << std::endl;
            }
        } ; 
    } // namespace B
    
    
} // namespace latte
