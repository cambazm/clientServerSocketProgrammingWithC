/*************************************
 * Bilgisayar Haberlesmesi Odev-2    *
 *				     *
 * Ogrenci : MEHMET CAMBAZ           *
 * No      : 040020365               *
 * E-mail  : mehmet_cambaz@yahoo.com *
 *************************************/

/***istemci***/

/*program yazilirken temel olarak derste gosterilen ornek kodlar kullanilmistir*/

#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

#define STDIN_FILENO	0
#define BUFFSIZE	255
#define ISIMBOYU	15
#define PROTOPORT       5193            /* varsayilan protokol port numarasi */
#define QLEN            6               /* istek kuyruk uzunlugu */

extern  int             errno;
char    localhost[] =   "localhost";    /* varsayilan host adi */

int readln(char *, int);
void basarisiz_recv();
void dosyada_sorun();
void menu();

/************************************************************************
* Fonksiyon adi: main 						 	*
* Amac: isteklere gore istemcinin yapmasi gerekenleri gerceklemek	*
* Giris parametreleri: sunucu ipsi ve protokol port nosu	 	*
* Cikis parametreleri: 0 (basarili sonlanma) veya 1 (basarisiz sonlanma)*
* Algoritma: iteratif algoritma					 	*
*************************************************************************/
int main(int argc, char *argv[]) 
{
        struct  hostent  *ptrh, *ptrh2; /* host tablosu */
        struct  protoent *ptrp, *ptrp2; /* protokol tablosu  */
        struct  sockaddr_in sad, sad2; 	/* IP adresi tutmak icin struct */
        struct  sockaddr_in cad, cad2; 	/* istemci adresi tutmak icin struct  */
        int     sd, sd2;              	/* soket tanimlayicisi */
        int     port;            	/* protokol port numarasi */
        char    *host;           	/* host ismi */
        int     n, n2, t;        
	int     alen;            	/* adres uzunlugu */
        char    buf[BUFFSIZE], mesaj[BUFFSIZE];
	char	isim[ISIMBOYU], ip[ISIMBOYU], yeniisim[ISIMBOYU], to[ISIMBOYU], tempisim[ISIMBOYU];
	int	i, secim=0, tmm=0;
	FILE	*fp;
	char	dosya_ismi[] = "istemci.txt";

		
        memset((char *)&sad,0,sizeof(sad)); /* sockaddr struct ini temizle */
        sad.sin_family = AF_INET;           /* ailesini internet ata */

        /* arguman olarak port verilmezse varsayilan deger atanir */
        if (argc > 2) {
                port = atoi(argv[2]);  
        } else {
                port = PROTOPORT;       /* varsayilan port numarasini kullan */
		printf("\nvarsayilan port atandi: 5193\n");
        }
        if (port > 0)
                sad.sin_port = htons((u_short)port);
        else {
                fprintf(stderr,"yanlis port numarasi %s\n",argv[2]);
                exit(1);
        }

        /* sunucu adi/ipsi arguman olarak verilmezse varsayilan atanir */
        if (argc > 1) {
                host = argv[1];
        } else {
                host = localhost;
		printf("varsayilan host atandi: localhost\n\n");
        }

        /* sokette ip kismini host un ip adresiyle doldur */
        ptrh = gethostbyname(host);
        if ( ((char *)ptrh) == NULL ) {
                fprintf(stderr,"gecersiz host: %s\n", host);
                exit(1);
        }
        memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

        /* protokol numarasini ilistir */
        if ( ((int)(ptrp = getprotobyname("tcp"))) == 0) {
                fprintf(stderr, " \"tcp\" protokole ilistirilemedi");
                exit(1);
        }

        /* soket olustur */
        sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
        if (sd < 0) {
                fprintf(stderr, "soket olusturulmasi basarisiz\n");
                exit(1);
        }

        /* belirtilen sunucuya baglan */
        if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
                fprintf(stderr,"baglanti kurulamadi\n");
                exit(1);
        }

		/****KULLANICI ISMI AL****/
		printf("Istemci ismi (en fazla 15 karakter)\n: ");
		n = readln(isim, ISIMBOYU);	//kullancdan isim al
		isim[n-1] = '\0';

		//islem = 1 : kayit olma
		buf[0] = '1';	//buf in ilk elemani islem no
		
		//buf in ilk elemanindan sonraki kismi kullanici ismi
		for(i=1;i<n+1;i++)
			buf[i]=isim[i-1];
		buf[n+1]='\0';

		send(sd,buf,strlen(buf),0);		//sunucuya yolla
		buf[0] = '\0';

		n = recv(sd, buf, sizeof(buf), 0);
		/*MEVCUT ISIMLERI SUNUCUDAN AL, DOSYAYA YAZ*/
		if(n > 0) {
			/*eger hata paketi geldiyse istemci ismi kabul edilmemistir*/
			if(buf[0]=='6') {
				printf("Hata: Boyle bir istemci ismi mevcut\n");
				closesocket(sd);
				exit(1);
			}
			printf("\nAlinan isim: %s\n",isim);
			buf[n-1] = '\0';

			fp = fopen(dosya_ismi, "w");
			if(!fp)
				dosyada_sorun();

			for(i=0;i<n;i++)
				fprintf(fp, "%c", buf[i]);
		
			fclose(fp);
		}
		else
			basarisiz_recv();

		closesocket(sd);
		
		while(1) 
		{
			menu();
			fflush(stdin);
			scanf("%d", &secim);

			switch(secim)
			{
				//Kayit Guncelleme
				case 1:

					printf("Yeni istemci ismi (en fazla 15 karakter)\n: ");
					n = readln(yeniisim, ISIMBOYU);		//kullancdan isim al
					yeniisim[n-1] = '\0';

					sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
					if (sd < 0) {
						fprintf(stderr, "soket olusturulmasi basarisiz\n");
						continue;
					}
					if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
						fprintf(stderr,"baglanti kurulamadi\n");
						continue;
					}

					buf[0] = '2';
					
					n = strlen(yeniisim);
					
					for(i=1; i<n+1; i++)
						buf[i] = yeniisim[i-1];
		
					buf[n+1]='\0';

					send(sd,buf,strlen(buf),0);	//sunucuya kayit guncelleme istegini yolla

					n = recv(sd, buf, sizeof(buf), 0);	//dondurulen kayitlari al
					/*hata paketi dondurulmusse istemci ismi kabul edilmemistir*/
					if(buf[0]=='6') {
						printf("Isim mevcut\n");
						buf[0]='\0';
						closesocket(sd);
						break;
					}

					strcpy(isim, yeniisim);	//guncelleme sonrasi artik istemci bu adla anilacaktir

					//dosyayi guncelle
					if(n > 0) {

						buf[n] = '\0';

						fp = fopen(dosya_ismi, "w");
						if(!fp)
							dosyada_sorun();

						for(i=0;i<n;i++)
							fprintf(fp, "%c", buf[i]);
		
						fclose(fp);
					}
					else
						basarisiz_recv();
					
					break;

				//Kayitlari Sorgulama
				case 2:
										
					sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
					if (sd < 0) {
						fprintf(stderr, "soket olusturulmasi basarisiz\n");
						continue;
					}
					if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
						fprintf(stderr,"baglanti kurulamadi\n");
						continue;
					}

					buf[0] = '3';
					buf[1] = '\0';
					send(sd,buf,strlen(buf),0);	//sunucuya kayit sorgulama istegi yolla
					
					n = recv(sd, buf, sizeof(buf), 0);	//dondurulen kayitlari al
					//dosyayi guncelle
					if(n > 0) {

						buf[n] = '\0';

						fp = fopen(dosya_ismi, "w");
						if(!fp)
							dosyada_sorun();

						for(i=0;i<n;i++)
							fprintf(fp, "%c", buf[i]);
		
						fclose(fp);
						
						fp = fopen(dosya_ismi, "r");
						if(!fp)
							dosyada_sorun();

						printf("\nSistemde kayitli istemciler\n\n");

						while(fgets(isim,ISIMBOYU,fp) > 0) {
							printf("%s", isim);
							fgets(ip,ISIMBOYU,fp);
						}
						fclose(fp);

					}
					else
						basarisiz_recv();

					break;
				
				//Mesaj Gonderme
				case 3:
					/*gerekli tanimlamalar*/
					memset((char *)&sad2,0,sizeof(sad2)); /* clear sockaddr structure */
					sad2.sin_family = AF_INET;         /* set family to Internet     */
					sad2.sin_addr.s_addr = INADDR_ANY; /* set the local IP address   */

					port = PROTOPORT+3;
					sad2.sin_port = htons((u_short)port);

					tmm=0;
					printf("Mesaj gonderilmek istenen kisi\n: ");
					n = readln(to, ISIMBOYU);	//kullanicidan isim al
					to[n-1] = '\0';

					fp = fopen(dosya_ismi, "r");
					/*ilk once isme istemci kendi dosyasinda bakar*/
					while(fgets(tempisim,ISIMBOYU,fp) > 0) {
						tempisim[strlen(tempisim)-1]='\0';
						if(strcmp(tempisim,to)==0) {
							tmm = 1;
							/*isim bulundu, ip yi al*/
							fgets(ip,ISIMBOYU,fp);
							ip[strlen(ip)-1]='\0';

							fclose(fp);
							break;
						}
						fgets(ip,ISIMBOYU,fp);
					} 
					/*eger istemcinin kendi dosyasinda isim mevcutsa*/
					if(tmm==1) {
						/*yeni bir soket olusturularak mesajlasma onun uzerinden yapilir*/
						host = ip;
						ptrh2 = gethostbyname(host);
						if ( ((char *)ptrh2) == NULL ) {
							fprintf(stderr,"gecersiz host-1: %s\n", host);
							continue;
						}
						memcpy(&sad2.sin_addr, ptrh2->h_addr, ptrh2->h_length);

						if ( ((int)(ptrp2 = getprotobyname("tcp"))) == 0) {
							fprintf(stderr, " \"tcp\" protokole ilistirilemedi");
							continue;
						}

						sd2 = socket(PF_INET, SOCK_STREAM, ptrp2->p_proto);
						if (sd2 < 0) {
							fprintf(stderr, "soket olusturulmasi basarisiz\n");
							continue;
						}

						if (connect(sd2, (struct sockaddr *)&sad2, sizeof(sad2)) < 0) {
							fprintf(stderr,"baglanti kurulamadi\n");
							continue;
						}
						printf("Mesaj giriniz\n: ");
						n = readln(mesaj, BUFFSIZE);	//kullanicidan mesaj al
						mesaj[n-1] = '\0';
		
						send(sd2,mesaj,strlen(mesaj),0);	//mesaji yolla
						closesocket(sd2);

					}
					/*istemcinin kendi dosyasinda isim mevcut degilse*/
					else {					
						sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
						if (sd < 0) {
							fprintf(stderr, "soket olusturulmasi basarisiz\n");
							continue;
						}
						if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
							fprintf(stderr,"baglanti kurulamadi\n");
							continue;
						}

						fclose(fp);
						buf[0] = '4';
						for(i=1; i<strlen(to)+1; i++)
							buf[i] = to[i-1];
						buf[i] = '\0';

						send(sd,buf,strlen(buf),0);	//sunucuya kayit sorgulama istegi yolla	

						n = recv(sd, buf, sizeof(buf), 0);

						if(n > 0){

							if(buf[0] == '4') { //sunucuda bulundu
								for(i=1; i<n; i++)
									ip[i-1] = buf[i];
								ip[n-1] = '\0'; 

								/*yeni bir soket olusturularak mesajlasma onun uzerinden yapilir*/
								host = ip;
								ptrh2 = gethostbyname(host);
								if ( ((char *)ptrh2) == NULL ) {
									fprintf(stderr,"gecersiz host-2: \"%s\"\n", host);
									continue;
								}
								memcpy(&sad2.sin_addr, ptrh2->h_addr, ptrh2->h_length);
			
								if ( ((int)(ptrp2 = getprotobyname("tcp"))) == 0) {
									fprintf(stderr, " \"tcp\" protokole ilistirilemedi");
									continue;
								}
			
								sd2 = socket(PF_INET, SOCK_STREAM, ptrp2->p_proto);
								if (sd2 < 0) {
									fprintf(stderr, "soket olusturulmasi basarisiz\n");
									continue;
								}
		
								if (connect(sd2, (struct sockaddr *)&sad2, sizeof(sad2)) < 0) {
									fprintf(stderr,"baglanti kurulamadi\n");
									continue;
								}
								
								printf("Mesaj giriniz\n: ");
								n = readln(mesaj, BUFFSIZE);	//kullanicidan mesaji al
								mesaj[n-1] = '\0';
		
								send(sd2,mesaj,strlen(mesaj),0);	//mesaji yolla
								closesocket(sd2);
							}
							else if(buf[0] == '6') { //sunucuda da yok
								printf("Hata: Boyle bir isimde istemci mevcut degil.");
								break;
							}

						} // end of n > 0
						else
							basarisiz_recv();

					}
					break;

				//Mesaj Alma
				case 4:


					memset((char *)&sad2,0,sizeof(sad2));
					sad2.sin_family = AF_INET;         
					sad2.sin_addr.s_addr = INADDR_ANY; 


					port = PROTOPORT+3;
					/*istemci-istemci iletisiminde istemci-sunucu'da kullanildigindan farkli bir port kullanilmak zorundadir*/
					if (port > 0)
						sad2.sin_port = htons((u_short)port);
					else {
						fprintf(stderr,"yanlis port numarasi %s\n",argv[1]);
						continue;
					}

					if ( ((int)(ptrp2 = getprotobyname("tcp"))) == 0) {
						fprintf(stderr, " \"tcp\" protokole ilistirilemedi");
						continue;
					}

					sd2 = socket(PF_INET, SOCK_STREAM, ptrp2->p_proto);
					if (sd2 < 0) {
						fprintf(stderr, "soket olusturulmasi basarisiz\n");
						continue;
					}

					if (bind(sd2, (struct sockaddr *)&sad2, sizeof(sad2)) < 0) {
						fprintf(stderr,"baglama basarisiz\n");
						continue;
					}

					if (listen(sd2, QLEN) < 0) {
						fprintf(stderr,"dinleme basarisiz\n");
						continue;
					}
						alen = sizeof(cad2);
						if ( (sd2 = accept(sd, (struct sockaddr *)&cad2, &alen)) < 0) {
							fprintf(stderr, "kabul basarisiz\n");
							return 1;
						}
						n = recv(sd2, buf, sizeof(buf), 0);
						buf[n] = '\0';
						if(n > 0) {
							printf("%s ip sinden gelen mesaj: \n",inet_ntoa(cad2.sin_addr));
							printf("%s",buf);
						}

						closesocket(sd2);

					break;

				//Cikis
				case 5:
					closesocket(sd);
					return 0;

				default:
					printf("\nHATALI SECIM\n");
					break;
			}
			closesocket(sd);
		} // end of while

        closesocket(sd);

        return 0;
}

 /************************************************************************************
* Fonksiyon adi: readln 					 		     *
* Amac: maksimum belli bir uzunlukta kullanicidan \0 veya \n ile sonlanan giris almak*
* Giris parametreleri: girisin yazilacagi yerin baslangic adresi, maksimum boy	     *
* Cikis parametreleri: alinan katarin boyu					     *
* Algoritma: iteratif algoritma					 		     *
*************************************************************************************/
int readln(char *buff, int buffsz)
{
	char *bp = buff, c;
	int n;

	while(bp - buff < buffsz && 
	      (n = read(STDIN_FILENO, bp, 1)) > 0) {
		if (*bp++ == '\n')
			return (bp - buff);
	}

	if (n < 0)
		return -1;

	if (bp - buff == buffsz)
		while (read(STDIN_FILENO, &c, 1) > 0 && c != '\n');

	return (bp - buff);
}
/****************************************
* Fonksiyon adi: basarisiz_recv 	*
* Amac: hata bilgisi ekrana cikartmak	*
* Giris parametreleri: yok	 	*
* Cikis parametreleri: yok		*
****************************************/
void basarisiz_recv()
{
	printf("Bilgi alma islemi basarisiz\n");
}
/****************************************
* Fonksiyon adi: dosyada_sorun	 	*
* Amac: hata bilgisi ekrana cikartmak	*
* Giris parametreleri: yok	 	*
* Cikis parametreleri: yok		*
****************************************/
void dosyada_sorun()
{
	printf("Dosya olusturmada hata");
	exit(1);
}
/****************************************
* Fonksiyon adi: menu	 		*
* Amac: menu bilgisini ekrana cikartmak	*
* Giris parametreleri: yok	 	*
* Cikis parametreleri: yok		*
****************************************/
void menu()
{
	printf("\n--------------------------\n");
	printf("  1) Kayit Guncelleme\n");
	printf("  2) Kayitlari Sorgulama\n");
	printf("  3) Mesaj Gonderme\n");
	printf("  4) Mesaj Alma\n");
	printf("  5) Cikis\n");
	printf("--------------------------\nSecim: ");
}
