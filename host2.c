#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

typedef unsigned long ip_address;
typedef unsigned short uint16;

static const int BUFFER_SIZE = 16 * 1024;

int Send(uint16 port)
{
   int sock = 0, valread, client_fd;
   struct sockaddr_in serv_addr;
   char ip_address[32];
   char filename[200];

   // printf(("\nEnter ipv4 address of receiver : "));
   // scanf("%s", ip_address);
   printf(("\nEnter path and filename of file that to be send : "));
   scanf("%s", filename);

   printf("\n");

   FILE *fpIn = fopen(filename, "r");
   if (fpIn)
   {

      if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      {
         printf("\n Socket creation error \n");
         return -1;
      }

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = INADDR_ANY;
      serv_addr.sin_port = htons(port);

      // Convert IPv4 and IPv6 addresses from text to binary
      // form
      if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
      {
         printf(
             "\nInvalid address/ Address not supported \n");
         return -1;
      }

      if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                               sizeof(serv_addr))) < 0)
      {
         printf("\nConnection Failed \n");
         return -1;
      }

      char acknowledgement[5];
      printf("Connected to remote host, sending filename \n");

      send(sock, filename, strlen(filename), 0);

      // receiving acknowledgement
      valread = read(sock, acknowledgement, sizeof(acknowledgement));
      printf("%s\n", acknowledgement);

      if (strcmp(acknowledgement, "Done") == 0)
      {

         printf("Filename succesfully received by receiver.\n");
         char buf[BUFFER_SIZE];
         while (1)
         {
            ssize_t bytesRead = fread(buf, 1, sizeof(buf), fpIn);
            if (bytesRead <= 0)
               break; // EOF

            printf("Read %i bytes from file, sending them to network...\n", (int)bytesRead);
            if (send(sock, buf, bytesRead, 0) != bytesRead)
            {
               perror("send");
               break;
            }
         }
      }

      // sending file

      close(client_fd);
      shutdown(sock, SHUT_RDWR);
      // sending file
      //  SendFile(ip_address, port, fpIn);

      // closing the connected socket
      fclose(fpIn);
      return 1;
   }
   else
   {
      printf("Error, couldn't open file to send!\n");
      return -1;
   }
}

char *extractFileName(char filepath[])
{

   size_t size = strlen(filepath) / sizeof(filepath[0]);
   int countOfFilename = 0;
   for (int i = size - 1; i >= 0; i--)

   {
      if (filepath[i] == '/')
         return &filepath[i + 1];
   }
}


void Receive(uint16 port)
{

   int server_fd, new_socket, valread;
   struct sockaddr_in address;
  
   int opt = 1;
   int addrlen = sizeof(address);

   // Creating socket file descriptor
   if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket failed");
      exit(EXIT_FAILURE);
   }

   // Forcefully attaching socket to the port 8080
   if (setsockopt(server_fd, SOL_SOCKET,
                  SO_REUSEADDR | SO_REUSEPORT, &opt,
                  sizeof(opt)))
   {
      perror("setsockopt");
      exit(EXIT_FAILURE);
   }
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(port);

   // Forcefully attaching socket to the port 8080
   if (bind(server_fd, (struct sockaddr *)&address,
            sizeof(address)) < 0)
   {
      perror("bind failed");
      exit(EXIT_FAILURE);
   }
   if (listen(server_fd, 3) < 0)
   {
      perror("listen");
      exit(EXIT_FAILURE);
   }
   if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0)
   {
      perror("accept");
      exit(EXIT_FAILURE);
   }

   char filePath[200] = {0};
   char *fileName;
   valread = recv(new_socket,filePath, 200, 0);
   printf("%ld", strlen(filePath));

   // autual file name form file path
   fileName = extractFileName(filePath);
   printf("%s", fileName);
   // sending acknowledgement for ready to receiving file.
   char acknowledgment[5] = "Done";

   printf("\n%s\n",acknowledgment);
   send(new_socket, acknowledgment, strlen(acknowledgment), 0);

   FILE *fpIn = fopen(fileName, "w");
   if (fpIn)
   {
      char buf[BUFFER_SIZE];
      while (1)
      {
         ssize_t bytesReceived = read(new_socket, buf, sizeof(buf));
         if (bytesReceived < 0)
            perror("recv"); // network error?
         if (bytesReceived == 0)
            break; // sender closed connection, must be end of file

         printf("Received %i bytes from network, writing them to file...\n", (int)bytesReceived);
         if (fwrite(buf, 1, bytesReceived, fpIn) != (size_t)bytesReceived)
         {
            perror("fwrite");
            break;
         }
      }

      fclose(fpIn);
   }

   close(new_socket);
   // closing the listening socket
   shutdown(server_fd, SHUT_RDWR);
}

int main(int argc, char **argv)
{

   printf("\n\n1. Sending Filen\n");
   printf("2. Receiving File\n");
   printf("3. Exit\n");
   printf("Enter your choice :");

   char c;
   scanf("%c", &c);

   switch (c)
   {
   case '1':
      Send(9999);
      break;

   case '2':
      Receive( 9999);
      break;

   case '3':
      exit(0);

   default:
      break;
   }
   
   printf("Exiting, bye!\n");
   return 0;
}

//  /media/sf_SharedFolderVM/levi.jpeg