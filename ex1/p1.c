#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define F_SZ 8

#define PPP_FLAG 0x7E
#define PPP_ESC  0x7D
#define PPP_ADDR 0xFF
#define PPP_CTRL 0x03

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

// Added 'const' to parameters to prevent g++ warnings
int hash(const char url[]) {
    int sum = 0;
    for(int i = 0; url[i]; i++) sum += url[i];
    return sum % SIZE;
}

// Added 'const' to parameters
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

// Added 'const' to parameter
struct Node* search(const char url[]) {
    struct Node *tmp = table[hash(url)];
    while(tmp) {
        if(strcmp(tmp->url, url) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

// Added 'const' to parameter
void deleteEntry(const char url[]) {
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

// Added 'const'
void printIP(const char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByte(a); printByte(b); printByte(c); printByte(d);
    }
}

// Added 'const'
void printMAC(const char mac[]) {
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (int i = 0; i < 6; i++) printByte(x[i]);
    }
}

void showLayers(char msg[], int len, struct Node *dest) {
    printf("\nMessage :\n");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printf("\n\n");

    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printPort(srcPort); printf(" "); printPort(dest->port);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32);

    printf("========= Network Layer =========\n\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    printIP(srcIP); printf(" "); printIP(dest->ip);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64);

    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    printIP(srcIP); printf(" "); printIP(dest->ip); printf(" ");
    printMAC(srcMAC); printf(" "); printMAC(dest->mac);
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
            if(cur < len) { printByte(msg[cur]); printf(" "); }
            else { printByte(0); printf(" "); }
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}

void sender_PPP(unsigned char *data, int len) {
    FILE *fp = fopen("transmitted_ppp.txt", "w");
    if (!fp) return;

    printf("\n========= SENDER INTERMEDIATE PROCESSING (PPP) =========\n");
    fprintf(fp, "%02X ", PPP_FLAG);
    printf("-> Appending Start Flag      : "); printByte(PPP_FLAG); printf("\n");

    fprintf(fp, "%02X %02X ", PPP_ADDR, PPP_CTRL);
    printf("-> Appending Address Block   : "); printByte(PPP_ADDR); printf("\n");
    printf("-> Appending Control Block   : "); printByte(PPP_CTRL); printf("\n");

    fprintf(fp, "00 21 ");
    printf("-> Appending Protocol Ident. : "); printByte(0x00); printf(" "); printByte(0x21); printf("\n");

    printf("\nOriginal Payload (Binary):\n");
    for (int i = 0; i < len; i++) {
        printByte(data[i]); printf(" ");
    }
    printf("\n");

    printf("\n Stuffed Frame\n");
    printByte(PPP_FLAG); printf(" "); printByte(PPP_ADDR); printf(" ");
    printByte(PPP_CTRL); printf(" "); printByte(0x00); printf(" "); printByte(0x21); printf(" ");

    printf("\n-> Processing and Byte-Stuffing Payload Stream:\n   Final Frame Sequence: [ ");

    for (int i = 0; i < len; i++) {
        if (data[i] == PPP_FLAG || data[i] == PPP_ESC) {
            fprintf(fp, "%02X ", PPP_ESC);
            fprintf(fp, "%02X ", data[i]);
            printByte(PPP_ESC); printf(" "); printByte(data[i]); printf(" ");
        } else {
            fprintf(fp, "%02X ", data[i]);
            printByte(data[i]); printf(" ");
        }
    }
    printf("]\n");

    printByte(0x00); printf(" "); printByte(PPP_FLAG); printf("\n");

    fprintf(fp, "00 ");
    printf("-> Appending Checksum Field  : "); printByte(0x00); printf("\n");

    fprintf(fp, "%02X\n", PPP_FLAG);
    printf("-> Appending End Flag        : "); printByte(PPP_FLAG); printf("\n");
    printf("========================================================\n");

    fclose(fp);
}

void receiver_PPP() {
    FILE *fp = fopen("transmitted_ppp.txt", "r");
    if (!fp) { printf("Error reading transmission file!\n"); return; }

    printf("\n========= RECEIVER SIDE (PPP BYTE DESTUFFING) =========\n");
    unsigned int i;
    int y = 0;
    int index = 0;

    printf("Extracted Elements from PPP Frame:\n");
    while (fscanf(fp, "%x", &i) == 1) {
        if (index == 0 && i == PPP_FLAG) {
            printf("Start Flag Detected: "); printByte(i); printf("\n");
            index++;
            continue;
        }
        if (index == 1) {
            printf("Address Field       : "); printByte(i); printf("\n");
            index++;
            continue;
        }
        if (index == 2) {
            printf("Control Field       : "); printByte(i); printf("\n");
            index++;
            continue;
        }
        if (index == 3) {
            printf("Protocol Field H    : "); printByte(i); printf("\n");
            index++;
            continue;
        }
        if (index == 4) {
            printf("Protocol Field L    : "); printByte(i); printf("\n");
            printf("Reading Payload Data Stream: ");
            index++;
            continue;
        }
        if (i == PPP_FLAG) {
            printf("\nChecksum Field      : "); printByte(0x00); printf(" (Dropped from stream data)\n");
            printf("End Flag Detected   : "); printByte(i); printf("\n");
            break;
        }
        if (i == PPP_ESC) {
            y = 1;
            continue;
        }
        if (y) {
            y = 0;
        }
        printByte((unsigned char)i); printf(" ");
    }
    printf("\n=======================================================\n");
    fclose(fp);
}

int main() {
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0, m, cho;

    preload();

    while(1) {
        URLTable();
        printf("\n========= MAIN MENU (PPP PROTOCOL RUNNER) =========\n");
        printf("1. Hash Table Management\n");
        printf("2. Proceed to Data Framing & Run PPP\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &m) != 1) return 0;

        if (m == 1) {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back\n");
            scanf("%d", &cho);

            if (cho == 1) {
                char newUrl[50], newIp[20], newMac[20]; int newPort;
                printf("Enter URL: "); scanf("%49s", newUrl);
                printf("Enter IP: "); scanf("%19s", newIp);
                printf("Enter MAC: "); scanf("%19s", newMac);
                printf("Enter Port: "); scanf("%d", &newPort);
                insert(newUrl, newIp, newMac, newPort);
            } else if (cho == 2) {
                char delUrl[50];
                printf("Enter URL to delete: ");
                scanf("%49s", delUrl);
                deleteEntry(delUrl);
            }
        }
        else if (m == 2) {
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

    sender_PPP((unsigned char*)msg, len);
    receiver_PPP();

    return 0;
}
