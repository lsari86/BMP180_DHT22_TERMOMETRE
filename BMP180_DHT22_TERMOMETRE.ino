/*
BMP180
VIN - 5V
GND - GND
SCL - A5
SDA - A4

DHT22
 -----------
 |    O    |
 -----------
   |  |  |
 3.3V 2 GND

 
*/

#include <dht.h> //DHT sensörleri kitaplığını taslağımıza dahil ediyoruz.
#include <SFE_BMP180.h> //Basınç sensörü kitaplığını taslağımıza dahil ediyoruz.
#include <Wire.h> //Wire kütüphanesini taslağımıza dahil ediyoruz.

#define DHTPIN 2 //DHT22'nin veri pinini Arduino'daki hangi pine bağladığımızı belirtiyoruz.
#define ALTITUDE 100.0 
//Bulunduğunuz şehrin rakım değeri. 
//Istanbul 100. 
//Yaşadığınız şehrin rakımına buradan bakın:
//http://tr.wikipedia.org/wiki/T%C3%BCrkiye_il_merkez_rak%C4%B1mlar%C4%B1


SFE_BMP180 bmp180;
dht DHT;

int gecenzaman=0;
int basinckarsilastirma[24] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //İçinde 24 adet değişken saklayan bir Array yaratıyoruz. Bu değişkenlerin her biri saat başı basınç değerlerini tutacak.
int firsttime =0; //Mini Hava İstasyonu'nun ilk defa açılıp açılmadığını kontrol etmek için oluşturduğumuz değişken.
int toplam=0;
int ortalama=0;
int fark=0;
int tahmin;
int yonelim;

void setup(){
  Serial.begin(9600);
  if (bmp180.begin()){
    Serial.println("BMP180 Bağlanıldı");
  }else{
    Serial.println("BMP180 Bağlanılamadı\n\n");
  while(1);
  }
}

void loop() {
  int readData = DHT.read22(DHTPIN);  
  int nem = DHT.humidity; //DHT22 sensöründen nem okunuyor ve nem değişkenine yazılıyor.
  int sicaklik = DHT.temperature; //DHT22 sensöründen sıcaklık okunuyor ve nem değişkenine yazılıyor.
  int basinc = readPressure(); //BMP180 sensöründen basınç okunuyor ve basinc değişkenine yazılıyor.
    
  float sicaklikf = DHT.temperature; //Detaylı bir şekilde göstermek için bir de float (küsüratlı) şeklinde değişken yapıyoruz.
  float nemf = DHT.humidity;  //Detaylı bir şekilde göstermek için bir de float (küsüratlı) şeklinde değişken yapıyoruz.
  float basincf = readPressure(); //Detaylı bir şekilde göstermek için bir de float (küsüratlı) şeklinde değişken yapıyoruz.
  
  
  toplam = basinckarsilastirma[0] + basinckarsilastirma[1] + basinckarsilastirma[2] + basinckarsilastirma[3] + basinckarsilastirma[4] + basinckarsilastirma[5] + basinckarsilastirma[6] + basinckarsilastirma[7] + basinckarsilastirma[8] + basinckarsilastirma[9] + basinckarsilastirma[10] + basinckarsilastirma[11] + basinckarsilastirma[12] + basinckarsilastirma[13] + basinckarsilastirma[14] + basinckarsilastirma[15] + basinckarsilastirma[16] + basinckarsilastirma[17] + basinckarsilastirma[18] + basinckarsilastirma[19] + basinckarsilastirma[20] + basinckarsilastirma[21] + basinckarsilastirma[22] + basinckarsilastirma[23]; //Belirli aralıklarla ölçülen basınç değerlerini topluyoruz.
  ortalama = toplam / 24; //Sonra 3'e bölerek ortalamasını almış oluyoruz.
  fark = basinc - ortalama; //Daha sonra güncel basınç değerini hesaplamış olduğumuz ortalamadan çıkararak aradaki farkı buluyoruz.
  
  // DÜŞÜK BASINÇ UYARISI (KIRMIZI LED)
  if ((fark) <=-4) {   //4 hPa'lık düşüş olursa. 
    //digitalWrite(3, HIGH); //Kırmızı yanar.
    Serial.print("Ani basınç değişikliği.(4hPa)");
    Serial.println(fark);
    tahmin=3;
  }

  Serial.println("\n\n");
  
  //SICAKLIK GÖSTERİLİR...
  Serial.print("SICAKLIK :");
  Serial.print(sicaklikf);
  Serial.println(" C ");
  delay(2500);
     
  // NEM GÖSTERİLİR...
  Serial.print("NEM :");
  Serial.print(nemf);
  Serial.println(" % ");
  delay(2500);
      
  // BASINÇ GÖSTERİLİR...   
  Serial.print("BASINC :");
  Serial.print(basincf);
  Serial.println(" hPa ");
  delay(2500);
     
  //BASINÇ ORTALAMALARI
  Serial.println("BASINC (SON 24 SAAT)");
  Serial.print("ORTALAMA: ");
  Serial.println(ortalama);
    
  Serial.print("Guncel: ");
  Serial.println(basinc);
      
  Serial.print("Fark: ");
  Serial.print(fark);
  delay(5000);  

  ++gecenzaman; //'gecenzaman' değişkenin 1 artmasını söylüyoruz.
  //Yukarıdaki birçok delay komutu sebebiyle 'gecenzaman' isimli değişkenin 1 artması için 20 saniye geçmesi gerekiyor.
  //Bu sayede Arduino açık olduğundan bu yana geçen zamanı hesaplayabiliriz. 'gecenzaman' değişkeni 180 sayısına ulaşırsa
  //1 saat geçmiş, 360'a ulaşırsa 2 saat geçmiş olur.. Böyle devam ettirerek hesaplayabiliriz.

  switch (gecenzaman){  //Saatlik ölçümler...
    case 180:
    basinckarsilastirma[0] = basinc;
    break;

    case 360:
    basinckarsilastirma[1] = basinc;
    if ((basinckarsilastirma[0]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[0]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[0]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 540:
    basinckarsilastirma[2] = basinc;
    if ((basinckarsilastirma[1]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[1]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[1]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 720:
    basinckarsilastirma[3] = basinc;
    if ((basinckarsilastirma[2]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[2]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[2]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 900:
    basinckarsilastirma[4] = basinc;
    if ((basinckarsilastirma[3]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[3]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[3]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 1080:
    basinckarsilastirma[5] = basinc;
    if ((basinckarsilastirma[4]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[4]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[4]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 1260:
    basinckarsilastirma[6] = basinc;
    if ((basinckarsilastirma[5]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[5]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[5]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 1440:
    basinckarsilastirma[7] = basinc;
    if ((basinckarsilastirma[6]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[6]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[6]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 1620:
    basinckarsilastirma[8] = basinc;
    if ((basinckarsilastirma[7]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[7]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[7]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 1800:
    basinckarsilastirma[9] = basinc;
    if ((basinckarsilastirma[8]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[8]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[8]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 1980:
    basinckarsilastirma[10] = basinc;
    if ((basinckarsilastirma[9]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[9]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[9]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 2160:
    basinckarsilastirma[11] = basinc;
    if ((basinckarsilastirma[10]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[10]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[10]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 2340:
    basinckarsilastirma[12] = basinc;
    if ((basinckarsilastirma[11]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[11]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[11]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
   
    case 2520:
    basinckarsilastirma[13] = basinc;
    if ((basinckarsilastirma[12]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[12]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[12]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;

    case 2700:
    basinckarsilastirma[14] = basinc;
    if ((basinckarsilastirma[13]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[13]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[13]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 2880:
    basinckarsilastirma[15] = basinc;
    if ((basinckarsilastirma[14]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[14]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[14]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 3060:
    basinckarsilastirma[16] = basinc;
    if ((basinckarsilastirma[15]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[15]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[15]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 3240:
    basinckarsilastirma[17] = basinc;
    if ((basinckarsilastirma[16]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[16]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[16]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 3420:
    basinckarsilastirma[18] = basinc;
    if ((basinckarsilastirma[17]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[17]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[17]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 3600:
    basinckarsilastirma[19] = basinc;
    if ((basinckarsilastirma[18]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[18]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[18]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 3780:
    basinckarsilastirma[20] = basinc;
    if ((basinckarsilastirma[19]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[19]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[19]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 3960:
    basinckarsilastirma[21] = basinc;
    if ((basinckarsilastirma[20]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[20]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[20]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
    
    case 4140:
    basinckarsilastirma[22] = basinc;
    if ((basinckarsilastirma[21]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[21]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[21]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;

    case 4320:
    basinckarsilastirma[23] = basinc;
    if ((basinckarsilastirma[22]) > (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden düşükse bunu yönelim değişkenine yazıyoruz.
    yonelim=-1;
    }
    if ((basinckarsilastirma[22]) < (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerinden büyükse bunu yönelim değişkenine yazıyoruz.
    yonelim=1;
    }
    if ((basinckarsilastirma[22]) == (basinc)) { //Eğer bir önceki ölçüm, güncel basınç değerine eşitse bunu yönelim değişkenine yazıyoruz.
    yonelim=0;
    }
    break;
  } 
    

/////////////////////
}

//En başta girdiğimiz rakım bilgisini kullanarak, ham basınç değerleri deniz seviyesi basınç değerlerine dönüştürülüyor.
float readPressure()
{
  char status;
  double T,P,p0,a;

  status = bmp180.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = bmp180.getTemperature(T);
    if (status != 0)
    { 
      status = bmp180.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = bmp180.getPressure(P,T);
        if (status != 0)
        {
          p0 = bmp180.sealevel(P,ALTITUDE);       
          return p0;
        }
      }
    }
  }
}

