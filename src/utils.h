#ifndef ESPDMXNODE_UTILS_H
#define ESPDMXNODE_UTILS_H

inline uint8_t lerp(uint8_t start, uint8_t end, uint16_t step, uint16_t steps)
{
    return (end - start) * step / steps + start;
}

inline uint8_t scale(uint8_t value, float_t scale)
{
    return value * scale;
}

int sign(int value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

// Need this to convert mqtt payloads to ints -- the payload is not null terminated so atoi etc won't do
int str2int(const char* str, int len)
{
    int i;
    int ret = 0;
    for (i = 0; i < len; ++i)
        ret = ret * 10 + (str[i] - '0');
    return ret;
}

char *deblank(char *input)
{
    int i, j;
    char *output = input;
    for (i = 0, j = 0; (size_t)i < strlen(input); i++, j++)
    {
        if (input[i] != ' ')
            output[j] = input[i];
        else
            j--;
    }
    output[j] = 0;
    return output;
}

void parseNetAddr(uint8_t addrBuffer[], const char *_addrString)
{
    char addrString[16];
    strlcpy(addrString, _addrString, sizeof(addrString));

    char *tok;
    tok = strtok(addrString, ".");

    uint8_t idx = 0;
    while (tok != NULL && idx < 4)
    {
        addrBuffer[idx] = strtoul(tok, NULL, 0);
        tok = strtok(0, ".");
        idx++;
    }
}
char *printNetAddr(uint8_t addr[])
{
    static char buf[16];
    buf[0] = 0;
    if (addr[0] == 0 && addr[1] == 0 && addr[2] == 0 && addr[3] == 0)
        strlcpy(buf, "", sizeof(buf));
    else
        sprintf_P(buf, PSTR("%u.%u.%u.%u"), addr[0], addr[1], addr[2], addr[3]);
    return buf;
}

#endif
