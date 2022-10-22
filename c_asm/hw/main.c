/* 

최소 요구 사항 

aes 암호화 지원
모드: cbc, ecb
비트: 128, 192, 256

해쉬 지원
알고리즘: md5, sha1, sha256, sha512

단일 파일 암호화, 디렉토리 전체 암호화 지원

*/

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#define FLAG_INPUT      0x1
#define FLAG_OUTPUT     0x2
#define FLAG_ALGORITHM  0x4
#define FLAG_DECRYPT    0x8
#define FLAG_CIPHER     0x10

char *hash_list[] = {"MD5", "SHA1", "SHA256", "SHA512"};
char *cipher_list[] = {"AES-128-CBC", "AES-192-CBC", "AES-256-CBC", "AES-128-ECB", "AES-192-ECB", "AES-256-ECB"};

int file_exist(const char *path);
int encrypt_file(char *input, char *output, char *algorithm, int flag);
int check_algorithm(short unsigned int *flag, char *algorithm);
int encrypt(FILE *infp, FILE *outfp, const EVP_CIPHER *cipher,
            const unsigned char *key, const unsigned char *iv);
void usage();
void check_argv(int flag);
void create_dir(const char *output);
void encrypt_dir(char *input, char *output, char *algorithm, int flag);

int main(int argc, char* argv[]) {
    
    int c;
    char *input, *output;
    char *algorithm;
    unsigned short flag = 0x0;

    while( (c = getopt(argc, argv, "i:o:a:dh")) != -1 ) {
        switch(c) {
            case 'i':
                flag |= FLAG_INPUT;
                input = (char*)malloc(strlen(optarg)+1);
                strcpy(input, optarg);
                break;
            case 'o':
                flag |= FLAG_OUTPUT;
                output = (char*)malloc(strlen(optarg)+1);
                strcpy(output, optarg);
                break;
            case 'a':
                flag |= FLAG_ALGORITHM;
                algorithm = (char*)malloc(strlen(optarg)+1);
                strcpy(algorithm, optarg);
                break;
            case 'd':
                flag |= FLAG_DECRYPT;
                break;
            case '?':
            case 'h':
                usage();
        }
    }

    check_algorithm(&flag, algorithm);
    check_argv(flag);

    printf(
        "[+] Input file/path: %s\n" \
        "[+] Output file/Path: %s\n" \
        "[+] Selected Algorithm: %s\n" \
        "[+] Set Decrypt FLAG: %s\n"
        , input, output, algorithm, (flag & FLAG_DECRYPT) ? "Yes" : "No"
    );

    OpenSSL_add_all_algorithms();
    
    if (flag & FLAG_CIPHER){
        if ( file_exist(input) == 1 ) {
            encrypt_dir(input, output, algorithm, flag);
        }
        else {
            encrypt_file(input, output, algorithm, flag);
        }
    }

    return 0;

}

void usage() {

    printf( 
        "\n" \
        "usage: \n" \
        "    ./crypt -i [input file/path] -o [output file/path] -a [algorithm]\n" \
        "\n" \
        "-h print usage\n" \
        "-i [input file/path]\n" \
        "-o [output file/path]\n" \
        "-a [algorithm]\n" \
        "-d Set decrypt flag (Not supported HASH)\n" \
        "\n" \
        "Algorithms List:\n" \
    );

    printf("    HASH:\n");
    for (int i = 0; i < sizeof(hash_list)/8; i++) { printf("\t%s\n", hash_list[i]); }
    printf("    Cipher:\n");
    for (int i = 0; i < sizeof(cipher_list)/8; i++) { printf("\t%s\n", cipher_list[i]); }

    exit(-1);

}

void check_argv(int flag) {
    
    if ( flag != ( FLAG_INPUT | FLAG_OUTPUT | FLAG_ALGORITHM | (flag & FLAG_DECRYPT) | (flag & FLAG_CIPHER) ) ) {
        usage();
    }

    if ( flag & FLAG_DECRYPT ) {
        if ( !(flag & FLAG_CIPHER) ) {
            printf("[-] Hash algorithm does not support Decrypt\n");
            usage();
        }
    }

}

int check_algorithm(short unsigned int *flag, char *algorithm) {

    for ( int i = 0; i < sizeof(hash_list)/8; i++ ) {
        if(strcmp(algorithm, hash_list[i]) == 0) {
            return 0;
        }
    }

    for ( int i = 0; i < sizeof(cipher_list)/8; i++ ) {   
        if(strcmp(algorithm, cipher_list[i]) == 0) {
            *flag |= FLAG_CIPHER;
            return 0;
        }
    }

    printf("[-] Checking the Algorithm");
    usage();

}

int file_exist(const char *path) {

    struct stat statbuf;

    if( ( stat(path, &statbuf ) ) < 0 ) {
        fprintf(stderr, "\n[-] does not exist file/dir name: %s\n", path);
        exit(-1);
    }

    return S_ISDIR(statbuf.st_mode);

}

int encrypt(FILE *infp, FILE *outfp, const EVP_CIPHER *cipher,
            const unsigned char *key, const unsigned char *iv) {

    int inlen, outlen;
    char inbuf[BUFSIZ], outbuf[BUFSIZ+EVP_MAX_BLOCK_LENGTH];
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    EVP_CIPHER_CTX_init(ctx);

    EVP_EncryptInit_ex(ctx, cipher, NULL, key, iv);

    while(( inlen=fread(inbuf, 1, sizeof(inbuf), infp)) > 0) {
        if(!EVP_EncryptUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            printf("EVP_EncryptUpdate() error. \n");
            EVP_CIPHER_CTX_cleanup(ctx);
            return -1;
        }
        fwrite(outbuf, 1, outlen, outfp);
    }

    if(!EVP_EncryptFinal_ex(ctx, outbuf, &outlen)) {
        printf("EVP_EncryptFinal_ex() error. \n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return -2;
    }
    fwrite(outbuf, 1, outlen, outfp);

    EVP_CIPHER_CTX_cleanup(ctx);

    return 0;

}

int decrypt(FILE *infp, FILE *outfp, const EVP_CIPHER *cipher,
            const unsigned char *key, const unsigned char *iv) {

    int inlen, outlen;
    char inbuf[BUFSIZ], outbuf[BUFSIZ+EVP_MAX_BLOCK_LENGTH];
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    EVP_CIPHER_CTX_init(ctx);

    EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv);

    while(( inlen=fread(inbuf, 1, sizeof(inbuf), infp)) > 0) {
        if(!EVP_DecryptUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            printf("EVP_DecryptUpdate() error. \n");
            EVP_CIPHER_CTX_cleanup(ctx);
            return -1;
        }
        fwrite(outbuf, 1, outlen, outfp);
    }

    if(!EVP_DecryptFinal_ex(ctx, outbuf, &outlen)) {
        printf("EVP_DecryptFinal_ex() error. \n");
        EVP_CIPHER_CTX_cleanup(ctx);
        return -2;
    }
    fwrite(outbuf, 1, outlen, outfp);

    EVP_CIPHER_CTX_cleanup(ctx);

    return 0;

}

int encrypt_file(char *input, char *output, char *algorithm, int flag) {

    FILE *infp, *outfp;
    int opt;
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char iv[EVP_MAX_IV_LENGTH];
    const EVP_CIPHER *cipher = EVP_enc_null();

    memset(key, 0xc0c0, 32);
    memset(iv , 0xb311, 32);

    if((infp = fopen(input, "rb")) == NULL){
        printf("fopen \"%s\" error. \n", input);
    }

    if((outfp = fopen(output, "wb")) == NULL){
        printf("fopen \"%s\" error. \n", output);
    }
    
    cipher = EVP_get_cipherbyname(algorithm);

    if (!( flag & FLAG_DECRYPT )) {
        encrypt( infp, outfp, cipher, key, iv );
    }
    else {
        decrypt( infp, outfp, cipher, key, iv );
    }

    fclose(infp);
    fclose(outfp);

    return 0;

}

void create_dir(const char *output) {

    struct stat st;

    if (stat(output, &st) == -1) {
        mkdir(output, 0755);
    }

}

void encrypt_dir(char *input, char *output, char *algorithm, int flag) {

    DIR *directory;
    char *file_input, *file_output;
    struct dirent *dir_entry;

    create_dir(output);
    directory = opendir(input);

    while( 1 ) {

        dir_entry = readdir(directory);

        if (dir_entry == 0 ) { break; }
        if ( strncmp( dir_entry->d_name, ".", 1 ) == 0 || strncmp( dir_entry->d_name, "..", 1 ) == 0 ) { continue; }

        file_input = (char*)malloc(strlen(dir_entry->d_name) + strlen(input) + 8);
        file_output = (char*)malloc(strlen(dir_entry->d_name) + strlen(output) + 8);
        sprintf(file_input, "%s/%s", input, dir_entry->d_name);
        if ( ( flag & FLAG_DECRYPT ) ) { sprintf(file_output, "%s/%s_dec", output, dir_entry->d_name); }
        else { sprintf(file_output, "%s/%s_enc", output, dir_entry->d_name); }

        if ( file_exist(file_input) == 1 ) {}
        else {
            // printf("1");
            encrypt_file(file_input, file_output, algorithm, flag);
        }

        free(file_input);
        free(file_output);

    }
}