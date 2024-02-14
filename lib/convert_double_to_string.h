#ifndef DoubleToString_H
#define DoubleToString_H

char* doubleToString(double num) {
    // Allocate memory for the string
    char* str = (char*)malloc(20 * sizeof(char)); // Assuming max 20 characters for the double

    // Convert the double to a string
    sprintf(str, "%f", num);

    return str;
}
#endif