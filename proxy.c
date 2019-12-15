 #include <sys/socket.h>  
 #include <sys/types.h>  
 #include <resolv.h>  
 #include <string.h>  
 #include <stdlib.h>  
 #include <pthread.h>  
 #include<unistd.h>  
 #include<netdb.h> //hostent  
 #include<arpa/inet.h>  
 #include <errno.h>

//Based on http://amritapnitc.blogspot.com/2015/07/simple-proxy-server-in-c-using-multi.html
 int hostname_to_ip(char * , char *);  
 // A structure to maintain client fd, and server ip address and port address  
 // client will establish connection to server using given IP and port   
 struct serverInfo  
 {  
      int client_fd;  
      int server_fd;
      char ip[100];  
      char port[100];  
 };  
 // A thread function  
   
 void* client_to_server(void* vargp){
       struct serverInfo *info = (struct serverInfo *)vargp;  
        char buffer[65535];  
        int bytes =0;
            while(1)  
      {  
           //receive data from client  
           memset(&buffer, '\0', sizeof(buffer));  
           bytes = read(info->client_fd, buffer, sizeof(buffer));  
           if(bytes <= 0)  
           {  
           }  
           else   
           {  
                // send data to main server  
                write(info->server_fd,buffer,bytes);
                //write(server_fd, buffer, sizeof(buffer));  
                //printf("client fd is : %d\n",c_fd);                    
                printf("From client %d: ",info->client_fd);                    
                fputs(buffer,stdout);  
                printf("\n");    
                  fflush(stdout);  
           }  
      }
 }



// A thread for each client request
 void *runSocket(void *vargp)  
 {  
   struct serverInfo *info = (struct serverInfo *)vargp;  
   char buffer[65535];  
   int bytes =0;  
      printf("client:%d\n",info->client_fd);  
      printf("%s\n",info->ip);
      printf("%s\n",info->port); 
      //code to connect to main server via this proxy server  
      int server_fd =0;  
      struct sockaddr_in server_sd;  
      // create a socket  
      info->server_fd = socket(AF_INET, SOCK_STREAM, 0);  
      if(server_fd < 0)  
      {  
           printf("server socket not created\n");  
      }  
      printf("server socket created\n");       
      memset(&server_sd, 0, sizeof(server_sd));  
      // set socket variables  
      server_sd.sin_family = AF_INET;  
      server_sd.sin_port = htons(atoi(info->port));  
      server_sd.sin_addr.s_addr = inet_addr(info->ip);  
      //connect to main server from this proxy server  
      for(int i=1;i<11;i++){
           printf("Trying to connect to server...\n");
           printf("Try %d\n",i);
          if((connect(info->server_fd, (struct sockaddr *)&server_sd, sizeof(server_sd)))<0)  {
               fprintf(stderr,"Error: &d\n",errno);
               perror("Error printed by perror");
               printf("server connection not established\n");  
               if(i==10){
                    printf("Couldn't connect to server...\n");
                    printf("Exiting...\n");
                    exit(0);
               }
          }
          else{
                 printf("server socket connected\n"); 
                 break;
          }
          sleep(1);
      }
          pthread_t client_thread;
          pthread_create(&client_thread,0,client_to_server,(void*) info);
           //recieve response from server  
           while(1){
           memset(&buffer, '\0', sizeof(buffer));  
           bytes = read(info->server_fd, buffer, sizeof(buffer));  
           if(bytes <= 0)  
           {  
           }            
           else  
           {  
                // send response back to client  
                write(info->client_fd, buffer,sizeof(buffer));  
                printf("From server : ");                    
                fputs(buffer,stdout);   
                printf("\n");         
           }  
      };       
   return NULL;  
 }  
 // main entry point  
 int main(int argc,char *argv[])  
 {  
     pthread_t tid;  
     char port[100],ip[100];  
     char *hostname = argv[1];  
     char proxy_port[100];  
        // accept arguments from terminal  
        strcpy(ip,argv[1]); // server ip  
        strcpy(port,argv[2]);  // server port  
        strcpy(proxy_port,argv[3]); // proxy port  
        //hostname_to_ip(hostname , ip);  
        printf("server IP : %s and port %s \n" , ip,port);   
        printf("proxy port is %s \n",proxy_port);        
      //socket variables  
      int proxy_fd =0, client_fd=0;  
      struct sockaddr_in proxy_sd;  
 // add this line only if server exits when client exits  
 //signal(SIGPIPE,SIG_IGN);  
      // create a socket  
      if((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
      {  
          printf("\nFailed to create socket\n");  
      }  
      printf("Proxy created\n");  
      memset(&proxy_sd, 0, sizeof(proxy_sd));  
      // set socket variables  
      proxy_sd.sin_family = AF_INET;  
      proxy_sd.sin_port = htons(atoi(proxy_port));  
      proxy_sd.sin_addr.s_addr = INADDR_ANY;  
      // bind the socket  
      if((bind(proxy_fd, (struct sockaddr*)&proxy_sd,sizeof(proxy_sd))) < 0)  
      {  
           printf("Failed to bind a socket\n");  
      }  
      // start listening to the port for new connections  
      if((listen(proxy_fd, SOMAXCONN)) < 0)  
      {  
           printf("Failed to listen\n");  
      }  
      printf("waiting for connection..\n");  
      //accept all client connections continuously  
      while(1)  
      {  
           client_fd = accept(proxy_fd, (struct sockaddr*)NULL ,NULL);  
           printf("client no. %d connected\n",client_fd);  
           if(client_fd > 0)  
           {  
                 //multithreading variables      
                 struct serverInfo *item = malloc(sizeof(struct serverInfo));  
                 item->client_fd = client_fd;  
                 strcpy(item->ip,ip);  
                 strcpy(item->port,port);  
                 pthread_create(&tid, NULL, runSocket, (void *)item);  
                 sleep(1);  
           }  
      }  
      return 0;  
 }  
 int hostname_to_ip(char * hostname , char* ip)  
 {  
   struct hostent *he;  
   struct in_addr **addr_list;  
   int i;  
   if ( (he = gethostbyname( hostname ) ) == NULL)   
   {  
     // get the host info  
     herror("gethostbyname");  
     return 1;  
   }  
   addr_list = (struct in_addr **) he->h_addr_list;  
   for(i = 0; addr_list[i] != NULL; i++)   
   {  
     //Return the first one;  
     strcpy(ip , inet_ntoa(*addr_list[i]) );  
     return 0;  
   }  
   return 1;  
 }  
