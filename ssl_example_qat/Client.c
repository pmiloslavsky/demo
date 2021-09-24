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
    // TLS_client_method() TLSv1_2_client_method()
#if defined(QAT_INTEGRATION)
    method = TLSv1_2_client_method();  /* Create new client-method instance */
#else
    method = TLS_client_method();  /* Create new client-method instance */
#endif
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
#if defined(QAT_INTEGRATION)
    // QAT can only use TLS 1.2 for symmetric encryption (SSL_write)
    // This cipher suite is usable by QAT
    SSL_CTX_set_cipher_list(ctx, "AES256-SHA256");
    // Also needed for QAT and SSL 1.1.1
    printf("Disabling ENCRYPT_THEN_MAC for QAT\n");
    SSL_CTX_set_options(ctx,
                        SSL_OP_NO_ENCRYPT_THEN_MAC);
#endif
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
    //SSL_set_cipher_list(ssl, "ECDHE_RSA_AES_256_CBC_SHA256");
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {
        //To deal with >16b packets easier
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
      
        printf("Default:\n %s", SSL_get_cipher_list(ssl,0));
#if defined(PRINT_AVAILABLE_CIPHER_SUITES)
	STACK_OF(SSL_CIPHER) *stack;
	stack = SSL_get_ciphers(ssl);
	SSL_CIPHER *cipher; int bits;
	printf("Available:\n");
	while((cipher = (SSL_CIPHER*)sk_pop(stack)) != NULL)
	  {
	    printf("* Cipher: %s\n", SSL_CIPHER_get_name(cipher));
	    printf("  Bits: %i\n", SSL_CIPHER_get_bits(cipher, &bits));
	    printf("  Used bits: %i\n", bits);
	    printf("  Version: %s\n", SSL_CIPHER_get_version(cipher));
	    printf("  Description: %s\n", SSL_CIPHER_description(cipher, NULL, 0));
	  }
#endif

	
        printf("\nConnected with %s encryption %s\n", SSL_get_cipher(ssl), SSL_get_version(ssl));
	char buf[128]; SSL_CIPHER_description(SSL_get_current_cipher(ssl), buf, sizeof(buf));
	printf("%s\n",buf);
	ShowCerts(ssl);        /* get any certs */
	
	printf("Starting send/receive cycles of %d messages\n",cycles);
	
       for (length = 128; length <= MAX_PACKET; length=length*2)
       {
	 clock_gettime(CLOCK_REALTIME, &start);
	 
	 for (int ix=0; ix < cycles; ix++)
         {
	   bytes = SSL_write(ssl,wbuf,length);
	   if (bytes != length) {
	     int rc;
	     rc = SSL_get_error(ssl, bytes);
	     printf("Write failed: 0x%x  rc: 0x%x\n",bytes,rc);
	     ERR_print_errors_fp(stderr);
	     goto exitloop;}
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
