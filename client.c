#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

int main(int argc, char *argv[]) {


    int sockfd;
    sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
    char** body_data = (char**) malloc(2*sizeof(char*));
    int connected = 0;
    int acces_library = 0;
    char command[100];
    char** cookies = (char**)malloc(sizeof(char*));
    char token[BUFLEN];

    while (1) {
        
       fgets(command, 100, stdin);
       command[strlen(command) - 1] = '\0';
       
       if (strncmp(command, "exit", 4) == 0) {
           break;
       } else {
        sockfd = open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM, 0);
        if (strncmp (command, "register", 8) == 0) {
            int ok = 0;
            char username[100];
            char password[100];
            memset(username, 0, 100);
            memset(password, 0, 100);
            printf("username=");
            scanf("%s", username);
            printf("password=");
            scanf("%s", password);
            // Verificare spatii in nume si parola
            if (strstr(username, " ") != 0) {
                ok = 1;
            }
            if (strstr(password, " ") != 0) {
                ok = 1;
            }
            if (ok == 0) {
                JSON_Value *value = json_value_init_object();
                JSON_Object *object = json_value_get_object(value);
                json_object_set_string(object, "username", username);
                json_object_set_string(object, "password", password);
                char *serialized_string = NULL;
                serialized_string = json_serialize_to_string_pretty(value);

                
                body_data[0] = serialized_string;
                
                char *message = compute_post_request(IP_SERVER, "/api/v1/tema/auth/register",
                                "application/json",
                                body_data,
                                1,
                                NULL,
                                0,
                                NULL);

                char *response;
                send_to_server(sockfd, message);

                response = receive_from_server(sockfd);
                if(strstr(response, "is taken") != NULL) {
                    printf("400 - Try another username.\n");
                } else {
                    printf("200 - You are now registered.\n");
                }
                json_free_serialized_string(serialized_string);
                json_value_free(value);
                free(message);
            } else {
                printf ("402 - Wrong format, spaces not allowed\n");
            }
            
            
            
        } else

        if (strncmp (command, "login", 5) == 0) {
            if (connected == 0) {
                
                char username[100];
                char password[100];
                memset(username, 0, 100);
                memset(password, 0, 100);
                printf("username=");
                scanf("%s", username);
                printf("password=");
                scanf("%s", password);

                JSON_Value *value = json_value_init_object();
                JSON_Object *object = json_value_get_object(value);
                json_object_set_string(object, "username", username);
                json_object_set_string(object, "password", password);
                char *serialized_string = NULL;
                serialized_string = json_serialize_to_string_pretty(value);

                
                body_data[0] = serialized_string;

                char *message = compute_post_request(IP_SERVER,
                             "/api/v1/tema/auth/login","application/json",
                            body_data, 
                            1, 
                            NULL,
                            0,
                            NULL);

                char *response;
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                // Se cauta Cookie-ul
                if(strstr(response, "Set-Cookie: ") == NULL) {
                    printf("401 - Not reistered/Password inncorect.\n");
                } else {
                    response = strstr(response, "Set-Cookie: ");
                    strtok (response, ";");
                    response += 12;
                    cookies[0] = response;

                    if (response != NULL) {
                        connected = 1;
                        printf("201 - Logged in\n");
                    }
                }

                json_free_serialized_string(serialized_string);
                json_value_free(value);
                free(message);
            } else {
                printf("403 - Already logged in\n");
            }
        } else

        if (strncmp (command, "enter_library", 13) == 0) {
            if (connected) {
                char* message = compute_get_request(IP_SERVER,
                             "/api/v1/tema/library/access",
                            NULL,
                            cookies,
                            1,
                            NULL);

                send_to_server(sockfd, message);
                char* response;
                response = strstr (receive_from_server(sockfd), "token");
                // Se cauta Token-ul
                if (response != NULL) {
                    response = response + 8;
                    memset(token, 0, 200);
                    strcpy(token, response);
                    token[strlen(token) - 2] = '\0';
                    printf("202 - Welcome to the library\n");
                    acces_library = 1;

                }
                free(message);
            } else {
                printf("404 - Please log in\n");
            }
        } else

        if (strncmp (command, "add_book", 8) == 0) {
            if (connected) {
                if (acces_library) {
                    int page_count;
                    char title[100], author[100], genre[100], publisher[100];
                    memset(title, 0, 100);
                    memset(author, 0, 100);
                    memset(genre, 0, 100);
                    memset(publisher, 0, 100);
                    int ok = 0;
                    
                    printf("title=");
                    fgets(title, 100, stdin);
                    // Se verifica sa nu se introduca null
                    if (title[0] == '\n') {
                        ok = 1;
                    } else {
                        title[strlen(title) - 1] = '\0';
                    }
                    

                    printf("author=");
                    fgets(author, 100, stdin);
                    if (author[0] == '\n') {
                        ok = 1;
                    } else {
                        author[strlen(author) - 1] = '\0';
                    }

                    printf("genre=");
                    fgets(genre, 100, stdin);
                    if (genre[0] == '\n') {
                        ok = 1;
                    } else {
                        genre[strlen(genre) - 1] = '\0';
                    }

                    printf("publisher=");
                    fgets(publisher, 100, stdin);
                    if (publisher[0] == '\n') {
                        ok = 1;
                    } else {
                        publisher[strlen(publisher) - 1] = '\0';
                    }

                    printf("pages=");
                    // Se verifica numarul de pagini
                    scanf("%d", &page_count);
                    if (page_count < 1) {
                        ok = 2;
                    }

                    if (ok == 0) {
                        char pages[5];
                        sprintf(pages, "%d", page_count);

                        JSON_Value *value = json_value_init_object();
                        JSON_Object *object = json_value_get_object(value);
                        json_object_set_string(object, "title", title);
                        json_object_set_string(object, "author", author);
                        json_object_set_string(object, "genre", genre);
                        json_object_set_string(object, "page_count", pages);
                        json_object_set_string(object, "publisher", publisher);
                        char *serialized_string = NULL;
                        serialized_string = json_serialize_to_string_pretty(value);

                        
                        body_data[0] = serialized_string;
                        char *message = compute_post_request(IP_SERVER,
                                     "/api/v1/tema/library/books",
                                     "application/json",
                                    body_data,
                                    1,
                                    NULL,
                                    0,
                                    token);
                        char *response;
                        send_to_server(sockfd, message);
                        response = receive_from_server(sockfd);
                        if (strstr(response, "error") != NULL) {
                            printf("407 - Error\n");
                        } else {
                            printf("203 - Book added\n");
                        }
                        
                    } else {
                        if (ok == 1) {
                            printf("405 - Fields must not be null\n");
                        } else {
                            printf ("406 - Wrong format for pages\n");
                        }
                    }

                } else {
                    printf("408 - You don't have acces to the library.\n");
                }
            } else {
                printf("404 - Please log in\n");
            }
        } else

        if (strncmp (command, "get_books", 9) == 0) {
            if (connected) {
                if (acces_library) {
                    char* message = compute_get_request(IP_SERVER,
                                 "/api/v1/tema/library/books",
                                NULL,
                                cookies,
                                1,
                                token);

                    send_to_server(sockfd, message);
                    char* response;
                    // Raspunsul de la server
                    response = strstr (receive_from_server(sockfd), "[");
                    printf("%s\n", response);
                    free(message);
                } else {
                    printf("408 - You don't have acces to the library.\n");
                }
            } else {
                printf("404 - Please log in\n");
            }
        } else 

        if (strncmp (command, "get_book", 8) == 0) {
            if (connected) {
                if (acces_library) {
                    int book_id;
                    printf("id=");
                    scanf("%d", &book_id);
                    // verificare corectitudine id
                    while (book_id < 0) {
                        printf("409 - Wrong id. Try again.\n");
                        printf("id=");
                        scanf("%d", &book_id);
                    }
                    char Books[100];
                    memset(Books, 0, 100);
                    sprintf(Books, "/api/v1/tema/library/books/%d", book_id);
                    char* message = compute_get_request(IP_SERVER,
                                    Books,
                                    NULL,
                                    cookies,
                                    1,
                                    token);

                    send_to_server(sockfd, message);
                    char* response;
                    response = receive_from_server(sockfd);
                    if (strstr(response, "[") != NULL) {
                        response = strstr (response, "[");
                        response +=1;
                        response[strlen(response) - 1] = '\0';
                        printf("%s\n", response);
                    } else {
                        printf("410 - There's no book with this id\n");
                    }
                    free(message);

                } else {
                    printf("408 - You don't have acces to the library.\n");
                }
            } else {
                printf("404 - Please log in\n");
            }
        } else 

        if (strncmp (command, "delete_book", 11) == 0) {
            if (connected) {
                if (acces_library) {
                    int book_id;
                    printf("id=");
                    scanf("%d", &book_id);

                    while (book_id < 0) {
                        printf("409 - Wrong id. Try again.\n");
                        printf("id=");
                        scanf("%d", &book_id);
                    }
                    char Books[100];
                    memset(Books, 0, 100);
                    sprintf(Books, "/api/v1/tema/library/books/%d", book_id);
                    char* message = compute_delete_request(IP_SERVER,
                                    Books,
                                    NULL,
                                    cookies,
                                    1,
                                    token);

                    send_to_server(sockfd, message);
                    char* response;
                    response = receive_from_server(sockfd);
                    if (strstr(response, "No book") != NULL) {
                        printf("410 - There's no book with this id\n");
                    } else {
                        printf("206 - Book deleted\n");
                    }
                    free(message);

                } else {
                    printf("404 - You don't have acces to the library.\n");
                }
            } else {
                printf("404 - Please log in\n");
            }

        } else 

        if (strncmp (command, "logout", 6) == 0) {
            if (connected) {
                char* message = compute_get_request(IP_SERVER,
                                "/api/v1/tema/auth/logout",
                                NULL,
                                cookies, 1, token);
                send_to_server(sockfd, message);
                connected = 0;
                acces_library = 0;
                printf("207 - Disconnected\n");
                free(message);
            } else {
                printf ("404 - Please log in\n");
            }
        }
       }
        

    }
    // Eliberare memorie
    free(body_data);
    free(cookies);
    close_connection(sockfd);

    return 0;
}
