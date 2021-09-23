#include "transfer.h"

// Create the SSL socket and intialize the socket address structure
int OpenListener(int port)
{
    int sd;
    struct sockaddr_in addr;
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        perror("can't bind port");
        abort();
    }
    if ( listen(sd, 10) != 0 )
    {
        perror("Can't configure listening port");
        abort();
    }
    return sd;
}
int isRoot()
{
    if (getuid() != 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
SSL_CTX* InitServerCTX(void)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = TLS_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    // Test option for QAT and SSL 1.1.1
    printf("Disabling ENCRYPT_THEN_MAC for QAT\n");
    SSL_CTX_set_options(ctx,
                        SSL_OP_NO_ENCRYPT_THEN_MAC);
    return ctx;
}
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}
void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No peer certificates.\n");
}

void Servlet(SSL* ssl) /* Serve the connection -- threadable */
{
    //char wbuf[160000] = {0};
    int cycles = CYCLES; // messages of each size
    char rbuf[160000] = {0};
    size_t length = 0;
    struct timespec start, stop;
    double accum;
    int sd, bytes;

    if ( SSL_accept(ssl) == FAIL )     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    else
    {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
	ShowCerts(ssl);        /* get any certs */
	
	printf("Starting send/receive cycles of %d messages\n",cycles);
	
       for (length = 128; length <= MAX_PACKET; length=length*2)
       {
	 clock_gettime(CLOCK_REALTIME, &start);
	 
	 for (int ix=0; ix < cycles; ix++)
         {
	   bytes = SSL_read(ssl,rbuf,length);
	   if (bytes != length) {printf("Read failed: 0x%x\n",bytes); ERR_print_errors_fp(stderr); goto exitloop;}
	 }
	 clock_gettime(CLOCK_REALTIME, &stop);

	 accum = (stop.tv_sec - start.tv_sec)*(MICROS_IN_SEC)
	   + (stop.tv_nsec - start.tv_nsec)/NANOS_IN_MICRO;
	 printf("Time taken: %lf micros for %d reads of %d bytes,  bytes per second: %f\n",
		accum, cycles, (int)length,
		cycles*length/accum);
       }
	 
    }
exitloop:
    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */
}


int main(int count, char *Argc[])
{
    SSL_CTX *ctx;
    int server;
    char *portnum;
    //Only root user have the permsion to run the server
    if(!isRoot())
    {
        printf("This program must be run as root/sudo user!!");
        exit(0);
    }
    if ( count != 2 )
    {
        printf("Usage: %s <portnum>\n", Argc[0]);
        exit(0);
    }

#if defined(QAT_INTEGRATION)
		 int rc;
		 OPENSSL_load_builtin_modules();
		 ENGINE_load_builtin_engines();
		 rc = CONF_modules_load_file("/usr/local/ssl/ssl/openssl.cnf", NULL, 0);
		 if (rc <= 0) {
		    FILE *fpe = fopen("ssl_logfile_server", "a");
		    ERR_print_errors_fp(fpe);
		    fclose(fpe);
		    printf("Unable to load %s: 0x%x\n",
			   "/usr/local/ssl/ssl/openssl.cnf", rc); 
		 } else {
		   printf("Using %s\n", "/usr/local/ssl/ssl/openssl.cnf");
		 }
#endif
    // Initialize the SSL library
    SSL_library_init();
    portnum = Argc[1];
    ctx = InitServerCTX();        /* initialize SSL */
    LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs */
    server = OpenListener(atoi(portnum));    /* create server socket */
    while (1)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;
        int client = accept(server, (struct sockaddr*)&addr, &len);  /* accept connection as usual */
        printf("Connection: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx);              /* get new SSL state with context */
        SSL_set_fd(ssl, client);      /* set connection socket to SSL state */
        Servlet(ssl);         /* service connection */
	break;
    }
    close(server);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
}
