#include <stdint.h>
#include <unistd.h>

int main() {
    while(1) {
        close(-1);
    }
}
