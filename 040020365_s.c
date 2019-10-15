/*************************************
 * Bilgisayar Haberlesmesi Odev-2    *
 *				     *
 * Ogrenci : MEHMET CAMBAZ           *
 * No      : 040020365               *
 * E-mail  : mehmet_cambaz@yahoo.com *
 *************************************/

/***sunucu***/

/*program yazilirken temel olarak derste gosterilen ornek kodlar kullanilmistir*/

#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

#define BUFFSIZE	255
#define ISIMBOYU	15
#define PROTOPORT       5193            /* varsayilan protokol numarasi */
#define QLEN            6               /* istek kuyruk uzunlugu */


void dosyada_sorun();
void basarisiz_recv();

/****************************************************************************************
* Fonksiyon adi: main 						 			*
* Amac: istemci isteklerini karsilayarak en guncel istemci isimleri bilgisini tutmak	*
* Giris parametreleri: kullanilacak protokol port no	 				*
* Cikis parametreleri: 0 (basarili sonlanma) veya 1 (basarisiz sonlanma)		*
* Algoritma: iteratif algoritma					 			*
****************************************************************************************/
int main(int argc, char *argv[])
{
        struct  hostent  *ptrh;  /* host tablosu */
        struct  protoent *ptrp;  /* protokol tablosu  */
        struct  sockaddr_in sad; /* IP adresi tutmak icin struct */
        struct  sockaddr_in cad; /* istemci adresi tutmak icin struct  */
        int     sd, sd2;         /* soket tanimlayicisi */ 
        int     port;            /* protokol port numarasi */
        int     alen;            /* adres uzunlugu */
        char    buf[BUFFSIZE];   
	char	isim[ISIMBOYU], yeniisim[ISIMBOYU], ip[ISIMBOYU], yeniip[ISIMBOYU], tempisim[ISIMBOYU];
	int	i, j, n, tmm=0;
	char	c='-';
	FILE	*fp;
	char	dosya_ismi[] = "sunucu.txt";


        memset((char *)&sad,0,sizeof(sad)); /* sockaddr struct ini temizle */
        sad.sin_family = AF_INET;         /* ailesini internet ata */
        sad.sin_addr.s_addr = INADDR_ANY; 


	fp = fopen(dosya_ismi, "w");
	if(!fp)
		dosyada_sorun();

	fclose(fp);

	/* arguman olarak port verilmezse varsayilan deger atanir */
        if (argc > 1) { 
                port = atoi(argv[1]);   
        } else {
                port = PROTOPORT;
		printf("\nvarsayilan port atandi: 5193\n");     
        }
        if (port > 0)                  
                sad.sin_port = htons((u_short)port);
        else {                         
                fprintf(stderr,"yanlis port numarasi %s\n",argv[1]);
                exit(1);
        }
	
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


        if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
                fprintf(stderr,"baglama islemi basarisiz\n");
                exit(1);
        }


        if (listen(sd, QLEN) < 0) {
                fprintf(stderr,"dinleme islemi basarisiz\n");
                exit(1);
        }

	
        while (1) {
	    	tmm = 0;

	    	alen = sizeof(cad);
	    	/*istemcilerden baglanti bekle*/
            	if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
                	fprintf(stderr, "kabul islemi basarisiz\n");
                	return 1;
            	}
		n = recv(sd2, buf, sizeof(buf), 0);
		if(n > 0) {

		switch(buf[0]) {

			/*KAYIT EKLEME*/
			case '1':
				printf("\nkayit ekleniyor..");
				fflush(stdout);
				fp = fopen(dosya_ismi, "r");
				for(i=1;i<n;i++)
					isim[i-1] = buf[i];
				isim[n-1] = '\0';
				tmm = 1;
				while(fgets(tempisim,ISIMBOYU,fp) > 0) {
					tempisim[strlen(tempisim)-1]='\0';
					/*istenen isim mevcutsa*/
					if(strcmp(tempisim,isim)==0) {
						buf[0]='6';
						buf[1]='\0';
						send(sd2,buf,strlen(buf),0);	//istemciye hata paketi yolla
						fclose(fp);
						tmm = 0;
						break;
					}
					fgets(ip,ISIMBOYU,fp);
				}
				if(tmm==0)
					continue;	//hata pakedi yolladiysak baska islem yapilmayacak
					
				fclose(fp);

				fp = fopen(dosya_ismi, "a");
				if(!fp)
					dosyada_sorun();

				/*yeni istemci ismini ve ipsini dosyaya ekle*/
				for(i=1;i<n;i++)
					fprintf(fp, "%c", buf[i]);

				fprintf(fp,"%s","\n");

				fprintf(fp, "%s\n", inet_ntoa(cad.sin_addr));

				fclose(fp);
				
				/*KAYIT SONRASI MEVCUT DOSYA ICERIGI YOLLANMASI*/
				fp = fopen(dosya_ismi, "r");
				if(!fp)
					dosyada_sorun();

				i=0;
				do {
					c = fgetc(fp);
					buf[i] = c;
					i++;
				} while (c != EOF);
				buf[i-1] = '\0';	//en son yazilan EOF, onu dosyaya yazmamiza gerek yok


				send(sd2,buf,strlen(buf),0);	//istemciye mevcut dosya icerigini yolla

				fclose(fp);

				break;

			//KAYIT GUNCELLEME
			case '2':
				printf("\nkayit guncelleniyor..");
				fflush(stdout);
				i = 1;
				for(i=1; i<n; i++)
					yeniisim[i-1] = buf[i];

				yeniisim[n-1] = '\0';

				strcpy(yeniip,(char*)inet_ntoa(cad.sin_addr));

				fp = fopen(dosya_ismi, "r+");
				tmm = 1;
				while(fgets(tempisim,ISIMBOYU,fp) > 0) {
					tempisim[strlen(tempisim)-1]='\0';
					/*isimlerde istenen isim mevcutsa*/
					if(strcmp(tempisim,yeniisim)==0) {
						buf[0]='6';
						buf[1]='\0';
						send(sd2,buf,strlen(buf),0);	//istemciye hata paketi yolla
						fclose(fp);
						tmm = 0;
						break;
					}
					fgets(ip,ISIMBOYU,fp);
				}
				if(tmm == 0)
					continue;	//hata varsa devam etme

				/*yeni kullanici adini dosyaya ekle*/
				fp = fopen(dosya_ismi, "a");
				for(i=1;i<n;i++)
					fprintf(fp, "%c", buf[i]);
				fprintf(fp,"\n");
				fputs(yeniip,fp);
				fprintf(fp,"\n");

				fclose(fp);
				
				fp = fopen(dosya_ismi, "r");

				i=0;
				do {
					c = fgetc(fp);
					buf[i] = c;
					i++;
				} while (c != EOF);
				buf[i-1] = '\0';	//en son yazlan EOF, onu dosyaya yazmamza gerek yok

				send(sd2,buf,strlen(buf),0);	/*en son dosya icerigini istemciye yolla*/

				fclose(fp);

				break;

			//KAYIT SORGULAMA
			case '3':
				printf("\nkayitlar sorgulaniyor..");
				fflush(stdout);	
				fp = fopen(dosya_ismi, "r");
				if(!fp)
					dosyada_sorun();

				i=0;
				do {
					c = fgetc(fp);
					buf[i] = c;
					i++;
				} while (c != EOF);
				buf[i-1] = '\0';	//en son yazlan EOF, onu dosyaya yazmamza gerek yok

				send(sd2,buf,strlen(buf),0);	/*en son dosya icerigini istemciye yolla*/

				fclose(fp);
			
				break;

							
			//ISTEMCIDE BILINMEYEN ISIM MEVCUT MU
			case '4':
				for(i=1; i<n; i++)
					yeniisim[i-1] = buf[i];
				
				yeniisim[n-1] = '\0';
				printf("\n%s kaydi sorgulaniyor..",yeniisim);
				fflush(stdout);

				fp = fopen(dosya_ismi, "r");
				
				while(fgets(tempisim,ISIMBOYU,fp) > 0) {
					tempisim[strlen(tempisim)-1]='\0';
					/*dosyada aranan isim varsa*/
					if(strcmp(tempisim,yeniisim)==0) {
						buf[0]='4';
						fgets(ip,ISIMBOYU,fp);
						ip[strlen(ip)-1] = '\0';
						for(i=0; i<strlen(ip); i++)
							buf[i+1] = ip[i];

						tmm = 1;
						buf[i+1]='\0';

						send(sd2,buf,strlen(buf),0);	//isme ait ip yi iceren pakedi yolla

						fclose(fp);
						break;
					}
					fgets(ip,ISIMBOYU,fp);
				}
				/*dosyada isim bulunamazsa*/
				if(tmm == 0) {
					fclose(fp);
					buf[0]='6';
					buf[1]='\0';
					send(sd2,buf,strlen(buf),0);	//istemciye hata pakedi yolla
				}

				tmm=0;

				break;

			default:
				printf("\nHATA\n");
				break;
			}

		}
		else 
			basarisiz_recv();

		closesocket(sd2);
        } // end of while
		return 0;
}
/****************************************
* Fonksiyon adi: dosyada_sorun	 	*
* Amac: hata bilgisi ekrana cikartmak	*
* Giris parametreleri: yok	 	*
* Cikis parametreleri: yok		*
****************************************/
void dosyada_sorun()
{
	printf("Dosyaya erisimde hata");
	exit(1);
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
