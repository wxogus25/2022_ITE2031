/* Assembler code fragment for LC-2K */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
char labelList[65536][7];
char fillList[65536][7];
char opcodeList[10][5] = {"add",  "nor",  "lw",   "sw",   "beq",
                          "jalr", "halt", "noop", ".fill"};
int labelToValue[65536];
char temp[MAXLINELENGTH];
int lineCnt = 0;
int labelCnt = 0;
int is_second = 0;

void errorEval(char *text) {
    printf("[%05d] : Valid check error occur in %s\n", lineCnt, text);
    exit(1);
}

int checkLabel(char *lab) {
    // 라벨이 있으면 idx 리턴,
    // 없으면 -1 리턴
    for (int i = 0; i < labelCnt; i++) {
        if (!strcmp(lab, labelList[i])) return i;
    }
    return -1;
}

int pushLabel(char *lab) {
    // 라벨 유효성 체크는 이미 validate에서 진행했음
    strcpy(labelList[labelCnt], lab);
    labelToValue[labelCnt] = lineCnt;
    return ++labelCnt;
}

void isValidLabel(char *label) {
    // 길이가 6보다 큰 경우, 숫자로 시작하는 경우, 이미 있는 라벨인 경우 0 리턴,
    // 아니면 1 리턴
    if (strlen(label) != 0 && (strlen(label) > 6 || isdigit(label[0]) ||
                               (is_second == 0 && checkLabel(label) != -1)))
        errorEval("Label");
}

void isValidOpcode(char *opcode) {
    // 목록에 없는 opcode인 경우 0 리턴,
    // 아니면 1 리턴
    for (int i = 0; i < 9; i++)
        if (!strcmp(opcodeList[i], opcode)) return;
    errorEval("Opcode");
}

int isValidField(char *filed, int isOffset, int labelPossible) {
    // 해당 filed가 라벨인지 숫자인지 확인하고 둘 다 아니면 에러
    // 라벨인 경우 라벨의 값 리턴, 숫자인 경우 숫자 리턴
    // 해당 필드에 해당하는 비트보다 많이 쓰는 경우 에러
    int x, i;
    printf("%d\n", lineCnt);
    if (isalpha(filed[0]) && labelPossible) {
        isValidLabel(filed);
        x = checkLabel(filed);
        if (x == -1) errorEval("Filed");
        if (labelPossible == 1 && isOffset == 1)
            x = labelToValue[x] - lineCnt - 1;
        else
            x = labelToValue[x];
        printf("======%d\n", x);
    } else {
        for (i = 0; filed[i] != '\0'; i++)
            if (isalpha(filed[i])) errorEval("Filed");

        sscanf(filed, "%d", &x);
        sprintf(temp, "%d", x);
        if (strcmp(filed, temp)) errorEval("Filed");
    }

    if (isOffset == -1) {
        return x;
    } else if (isOffset == 1) {
        if (x >= -32768 && x <= 32767) return x;
    } else {
        if (x >= 0 && x <= 7) return x;
    }
    errorEval("Filed");
    return 0;
}

int isValidArgs(char *opcode, char *arg0, char *arg1, char *arg2) {
    // opcode에 따른 args가 유효하지 않으면 0 리턴,
    // 아니면 1 리턴
    int ans = 0;
    for (int i = 0; i < 9; i++) {
        if (!strcmp(opcodeList[i], opcode)) {
            if (i < 2) {
                ans |= isValidField(arg0, 0, 0) << 19;
                ans |= isValidField(arg1, 0, 0) << 16;
                ans |= isValidField(arg2, 0, 0);
                ans |= i << 22;
            } else if (i < 5) {
                ans |= isValidField(arg0, 0, 0) << 19;
                ans |= isValidField(arg1, 0, 0) << 16;
                if (i == 4)
                    ans |= (((1 << 16) - 1) & isValidField(arg2, 1, 1));
                else
                    ans |= (((1 << 16) - 1) & isValidField(arg2, 1, -1));
                ans |= i << 22;
            } else if (i < 6) {
                ans |= isValidField(arg0, 0, 0) << 19;
                ans |= isValidField(arg1, 0, 0) << 16;
                ans |= i << 22;
            } else if (i < 8) {
                ans |= i << 22;
            } else {
                ans |= isValidField(arg0, -1, 1);
            }
        }
    }
    return ans;
}

void validate(char *label, char *opcode, char *arg0, char *arg1, char *arg2) {
    // label, opcode 유효성 검사
    isValidLabel(label);
    isValidOpcode(opcode);
}

// use of undefined labels, duplicate labels, offsetFields that don't fit
// in 16 bits, and unrecognized opcodes.
// label 길이가 6보다 큰 경우, label이 숫자로 시작하는 경우

int main(int argc, char *argv[]) {
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }
    inFileString = argv[1];
    outFileString = argv[2];
    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        if (lineCnt == 65536) {
            puts("ERROR : memory overflow (lines > 65536)");
            exit(1);
        }
        validate(label, opcode, arg0, arg1, arg2);
        if (strlen(label)) pushLabel(label);
        lineCnt++;
    }
    rewind(inFilePtr);
    is_second = 1;
    lineCnt = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        fprintf(outFilePtr, "%d\n", isValidArgs(opcode, arg0, arg1, arg2));
        lineCnt++;
    }
    return (0);
}
/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2) {
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return (0);
    }
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("%s\n", line);
        printf("error: line too long\n");
        exit(1);
    }
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }
    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr,
           "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r "
           "]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);
    return (1);
}