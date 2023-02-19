#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
 
bool useconst = false;
bool zeroterminated = false;
bool usestatic = false;

int myfgetc(FILE *f)
{
    int c = fgetc(f);
    if (c == EOF && zeroterminated) {
        zeroterminated = false;
        return 0;
    }
    return c;
}
 
void process(const char *ifname, const char *ofname)
{
    FILE *ifile, *ofile;

    ifile = fopen(ifname, "rb");
    if (ifile == NULL) {
        perror("fopen");
        exit(errno);
    }

    ofile = fopen(ofname, "wb");
    if (ofile == NULL) {
        perror("fopen");
        exit(errno);
    }

    char buf[PATH_MAX], *p;
    const char *cp;

    // This is basically basename()
    if ((cp = strrchr(ifname, '/')) != NULL) {
        ++cp;
    } else {
        if ((cp = strrchr(ifname, '\\')) != NULL) ++cp;
        else cp = ifname;
    }
    strcpy(buf, cp);
    for (p = buf; *p != '\0'; ++p) {
        if (!isalnum(*p)) *p = '_';
    }

    fprintf(ofile, "%s%sunsigned char %s[] = {\n", usestatic ? "static " : "", useconst ? "const " : "", buf);
    int c, col = 1, bytes = 0;
    while ((c = myfgetc(ifile)) != EOF) {
        if (col >= 72) {
            fputc('\n', ofile);
            col = 1;
        }
        fprintf(ofile, "0x%.2x,", c);
        col += 6;
        bytes++;
    }

    fprintf(ofile, "\n};\n\n");
    fprintf(ofile, "int %s_bytes = %d;\n", buf, bytes);
 
    fclose(ifile);
    fclose(ofile);
}
 
void usage(void)
{
    fprintf(stderr, "usage: bin2c [-csz] <input_file> <output_file>\n");
    fprintf(stderr, "    -c    add 'const' to the definition\n");
    fprintf(stderr, "    -s    add 'static' to the definition\n");
    fprintf(stderr, "    -z    terminate the array with a zero (useful for embedded C strings)\n");
    exit(1);
}
 
int main(int argc, char **argv)
{
    int o;
    while ((o = getopt(argc, argv, "csz")) != -1) {
        switch(o) {
            case 'c':
                useconst = true;
                break;
            case 's':
                usestatic = true;
                break;
            case 'z':
                zeroterminated = true;
                break;
            case '?':
            default:
                usage();
        }
    }
    argc -= optind - 1;
    argv += optind - 1;

    if (argc != 3) {
        usage();
    }

    process(argv[1], argv[2]);
    return 0;
}