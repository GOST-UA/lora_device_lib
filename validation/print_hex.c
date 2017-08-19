#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void print_hex(const char *name, const uint8_t *data, size_t len)
{
    size_t i;
    int offset;
    
    printf("%s: %n", name, &offset);
    
    for(i=0; i < (50-offset); i++){
        
        printf(" ");
    }
    
    printf("\"");
    
    for(i=0; i < len; i++){
        
        printf("\\x%02X", data[i]);
    }
    
    printf("\"\n");
}
