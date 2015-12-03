#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <iostream>
 
int main(int argc, char **argv)
{
    struct sockaddr_in sa;
    SSL*     ssl;
    X509*    server_cert;
 
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = SSL_CTX_new (SSLv23_method());
 
    int sd = ::socket (AF_INET, SOCK_STREAM, 0);//create socket
    if (sd!=-1 && ctx!=NULL)
    {
        memset (&sa, '\0', sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_addr.s_addr = inet_addr ("216.58.219.206");   /* Server IP */
        sa.sin_port        = htons     (443);           /* Server Port number */
 
        int err = ::connect(sd, (struct sockaddr*) &sa, sizeof(sa));
        if (err!=-1)
        {
            ssl = SSL_new (ctx);
            if (ssl!=NULL)
            {
                SSL_set_fd(ssl, sd);
                err = SSL_connect(ssl);
                if (err!=-1)
                {
                    server_cert = SSL_get_peer_certificate(ssl);
                    if (server_cert!=NULL)
                    {
                        BIO * bio_out = BIO_new_file("output.pem", "w");
                        if (bio_out)
                        {
                            X509_print(bio_out, server_cert); //parsed
                            PEM_write_bio_X509(bio_out, server_cert);
                            BIO_free(bio_out);
                        }
                        X509_free (server_cert);
                    }
                }
                SSL_free (ssl);
            }
            ::close(sd);//close socket
        }
        else {
            std::cout << "Connection error " << err << std::endl;
        }
    }
    else{
        std::cout << "Error: " << std::endl;
    }
    SSL_CTX_free (ctx);
}
