#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define F_SZ 8

struct Node {
    char url[50], ip[20], mac[20];
    int port;
    struct Node *next;
};

struct Node *table[SIZE] = {NULL};

char srcURL[50] = "Default Source";
char srcIP[20] = "192.168.1.10";
char srcMAC[20] = "11:22:33:44:55:66";
int srcPort = 51309;

// Changed to const char*
int hash(const char url[]) {
    int sum = 0;
    for(int i = 0; url[i]; i++) sum += url[i];
    return sum % SIZE;
}

// Changed to const char*
void insert(const char url[], const char ip[], const char mac[], int port) {
    int idx = hash(url);
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(newNode->url, url);
    strcpy(newNode->ip, ip);
    strcpy(newNode->mac, mac);
    newNode->port = port;
    newNode->next = table[idx];
    table[idx] = newNode;
}

// Changed to const char*
struct Node* search(const char url[]) {
    struct Node *tmp = table[hash(url)];
    while(tmp) {
        if(strcmp(tmp->url, url) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

// Changed to const char*
void deleteURL(const char url[]) {
    int idx = hash(url);
    struct Node *tmp = table[idx];
    struct Node *prev = NULL;

    while (tmp != NULL && strcmp(tmp->url, url) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL) return;

    if (prev == NULL) {
        table[idx] = tmp->next;
    } else {
        prev->next = tmp->next;
    }
    free(tmp);
}

void URLTable() {
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-19s %-5s\n", "URL", "IP", "MAC", "PORT");
    for(int i = 0; i < SIZE; i++) {
        struct Node *tmp = table[i];
        while(tmp) {
            printf("%-20s %-18s %-19s %-5d\n", tmp->url, tmp->ip, tmp->mac, tmp->port);
            tmp = tmp->next;
        }
    }
    printf("===========================================\n");
}

void preload() {
    insert("www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25);
    insert("www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443);
    insert("www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 80);
    insert("www.google.com", "142.250.190.47", "AA:BB:CC:DD:EE:04", 443);
}

void printByte(unsigned char n) {
    for (int i = 7; i >= 0; i--) printf("%d", (n >> i) & 1);
}

void printPort(int port) {
    for (int i = 15; i >= 0; i--) printf("%d", (port >> i) & 1);
}

// Changed to const char*
void printIP(const char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByte(a); printByte(b); printByte(c); printByte(d);
    }
}

// Changed to const char*
void printMAC(const char mac[]) {
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (int i = 0; i < 6; i++) printByte(x[i]);
    }
}

void showLayers(char msg[], int len, struct Node *dest) {
    printf("\nMessage :\n");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printf("\n\n");

    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printPort(srcPort); printPort(dest->port);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32);

    printf("========= Network Layer =========\n\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printPort(srcPort); printPort(dest->port);
    printIP(srcIP); printIP(dest->ip);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64);

    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) printByte(msg[i]);
    printPort(srcPort); printPort(dest->port);
    printIP(srcIP); printIP(dest->ip);
    printMAC(srcMAC); printMAC(dest->mac);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64 + 96);
}

void showFrames(char msg[], int len, int totalFrames, struct Node *dest) {
    printf("====Frame Contents======\n");
    for(int i = 0; i < totalFrames; i++) {
        printf("\n-----------------------------------------\n");
        int packetNo = (i / 2) + 1;
        printf("Packet No : %d\n", packetNo);
        printf("Frame No  : %d\n", i + 1);
        printf("Source Port      : "); printPort(srcPort); printf("\n");
        printf("Destination Port : "); printPort(dest->port); printf("\n\n");
        printf("Source IP      : "); printIP(srcIP); printf("\n");
        printf("Destination IP : "); printIP(dest->ip); printf("\n");
        printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
        printf("Destination MAC: "); printMAC(dest->mac); printf("\n");

        printf("Frame Data     : ");
        for(int j = 0; j < F_SZ; j++) {
            int cur = (i * F_SZ) + j;
            if(cur < len) printByte(msg[cur]);
            else printByte(0);
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}

void senderBit(unsigned char *data, int len) {
    FILE *fp = fopen("transmitted_bits.txt", "w");
    if (!fp) return;

    printf("\n========= SENDER INTERMEDIATE PROCESSING (BIT FRAMING) =========\n");
    fprintf(fp, "01111110");
    printf("-> Emitting Start Flag          : 01111110\n");

    fprintf(fp, "10101010");
    printf("-> Emitting Control Header      : 10101010\n");

    printf("\nOriginal Payload (Binary):\n");
    for (int i = 0; i < len; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            printf("%d", (data[i] >> bit) & 1);
        }
        printf(" ");
    }

    printf("\n\n-> Bit-senderBit Payload Stream   :\n   [ ");
    int co = 0;
    for (int i = 0; i < len; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            int cb = (data[i] >> bit) & 1;
            fprintf(fp, "%d", cb);
            printf("%d", cb);
            if (cb == 1) {
                co++;
                if (co == 5) {
                    fprintf(fp, "0");
                    co = 0;
                }
            } else {
                co = 0;
            }
        }
    }
    printf(" ]\n");

    fprintf(fp, "00000000");
    printf("-> Emitting Trailer Checksum    : 00000000\n");
    fprintf(fp, "01111110\n");
    printf("-> Emitting End Flag            : 01111110\n");
    printf("================================================================\n");
    fclose(fp);
}

void receiverBit() {
    FILE *fp = fopen("transmitted_bits.txt", "r");
    if (!fp) return;

    printf("\n========= RECEIVER SIDE (BIT receiverBit) =========\n");
    static char stream[32000];
    if (fscanf(fp, "%31999s", stream) != 1) {
        fclose(fp);
        return;
    }
    fclose(fp);

    int len = strlen(stream);
    if (len < 32) {
        printf("Error: Invalid Frame!\n");
        return;
    }

    printf("Start Flag Field   : 01111110\n");
    printf("Header Field Block : 10101010\n");
    printf("Destuffed Payload Data Binary Stream:\n");

    int sIdx = 16;
    int eIdx = len - 16;
    int co = 0;
    int bc = 0;

    for (int i = sIdx; i < eIdx; i++) {
        int bit = stream[i] - '0';
        if (bit == 1) {
            printf("1");
            co++;
            bc++;
        } else {
            if (co == 5) {
                co = 0;
                continue;
            }
            printf("0");
            co = 0;
            bc++;
        }
        if (bc == 8) {
            printf(" ");
            bc = 0;
        }
    }
    printf("\nTrailer Block Field: 00000000\n");
    printf("End Flag Field     : 01111110\n");
    printf("==================================================\n");
}

int main() {
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0, m, cho;

    preload();

    while(1) {
        URLTable();
        printf("\n========= MAIN MENU (BIT senderBit RUNNER) =========\n");
        printf("1. Hash Table Management\n");
        printf("2. Proceed to Data Framing & Run Bit senderBit\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &m) != 1) return 0;

        if (m == 1) {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back\n");
            if(scanf("%d", &cho) != 1) continue;

            if (cho == 1) {
                char newUrl[50], newIp[20], newMac[20]; int newPort;
                printf("Enter URL: "); scanf("%49s", newUrl);
                printf("Enter IP: "); scanf("%19s", newIp);
                printf("Enter MAC: "); scanf("%19s", newMac);
                printf("Enter Port: "); scanf("%d", &newPort);
                insert(newUrl, newIp, newMac, newPort);
            } else if (cho == 2) {
                char delUrl[50];
                printf("Enter URL to delete: "); scanf("%49s", delUrl);
                deleteURL(delUrl);
            }
        } else if (m == 2) {
            break;
        } else {
            return 0;
        }
    }

    printf("\nEnter Source URL from table: ");
    scanf("%99s", url);
    struct Node *srcNode = search(url);
    if(srcNode) {
        strcpy(srcURL, srcNode->url);
        strcpy(srcIP, srcNode->ip);
        strcpy(srcMAC, srcNode->mac);
        srcPort = srcNode->port;
    }

    printf("Enter Destination URL: ");
    scanf("%99s", url);
    struct Node *dest = search(url);
    if(!dest) return 0;

    printf("Enter File Name: ");
    scanf("%49s", fn);

    fp = fopen(fn, "r");
    if(!fp) return 0;

    while((ch = fgetc(fp)) != EOF && idx < 999) msg[idx++] = (char)ch;
    msg[idx] = '\0';
    fclose(fp);

    int len = strlen(msg);
    int totalFrames = len / F_SZ + (len % F_SZ != 0);

    showLayers(msg, len, dest);
    showFrames(msg, len, totalFrames, dest);

    senderBit((unsigned char*)msg, len);
    receiverBit();

    return 0;
}
