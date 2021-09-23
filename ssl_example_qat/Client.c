#include "transfer.h"

int OpenConnection(const char *hostname, int port)
{
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}
SSL_CTX* InitCTX(void)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLS_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
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
void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No peer client certificates configured.\n");
}
int main(int count, char *strings[])
{
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char wbuf[160000] = {0};
    int cycles = CYCLES; // messages of each size
    //char rbuf[160000] = {0};
    size_t length = 0;
    int bytes;
    char *hostname, *portnum;
    struct timespec start, stop;
    double accum;
    if ( count != 3 )
    {
        printf("usage: %s <hostname> <portnum>\n", strings[0]);
        exit(0);
    }

#if defined(QAT_INTEGRATION)
		 int rc;
		 OPENSSL_load_builtin_modules();
		 ENGINE_load_builtin_engines();
		 rc = CONF_modules_load_file("/usr/local/ssl/ssl/openssl.cnf", NULL, 0);
		 if (rc <= 0) {
		    FILE *fpe = fopen("ssl_logfile_client", "a");
		    ERR_print_errors_fp(fpe);
		    fclose(fpe);
		    printf("Unable to load %s: 0x%x\n",
			   "/usr/local/ssl/ssl/openssl.cnf", rc); 
		 } else {
		   printf("Using %s\n", "/usr/local/ssl/ssl/openssl.cnf");
		 }
#endif
    
    SSL_library_init();
    hostname=strings[1];
    portnum=strings[2];
    ctx = InitCTX();
    server = OpenConnection(hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
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
	   bytes = SSL_write(ssl,wbuf,length);
	   if (bytes != length) {printf("Write failed: 0x%x\n",bytes); ERR_print_errors_fp(stderr); goto exitloop;}
	 }
	 clock_gettime(CLOCK_REALTIME, &stop);

	 accum = (stop.tv_sec - start.tv_sec)*(MICROS_IN_SEC)
	   + (stop.tv_nsec - start.tv_nsec)/NANOS_IN_MICRO;
	 printf("Time taken: %lf micros for %d writes of %d bytes,  bytes per second: %f\n",
		accum, cycles, (int)length,
		cycles*length/accum);
	 
       }
       sleep(1); //dont close before other guy done reading
exitloop:
       SSL_free(ssl);        /* release connection state */
    }
    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
}
