#include "1_triangle/epic_triangle.h"
#include <iostream>
#include <string>

int main() 
{
    std::cout << "🔸 Tucoff's Vulkan Sandbox 🔸\n";
    std::cout << "Projects sumary:\n";
    std::cout << "  1. Triangle\n";
    std::cout << "Digit the number of the projet to show: ";

    int n = 0;
    std::cin >> n;

    switch (n) {
        case 1:
            triangle();
            break;
        default:
            std::cerr << "Projeto inválido. Nada será executado.\n";
            return 1;
    }

    return 0;
}