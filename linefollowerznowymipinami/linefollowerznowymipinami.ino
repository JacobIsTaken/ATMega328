// --- PINY BEZ ZMIAN ---
int m_prawy_A = 8; int m_prawy_B = 9;
int m_lewy_A = 2; int m_lewy_B = 3; // poprawka pinów
int en_prawy = 5;  int en_lewy = 6;

int czujniki[] = {A0, A1, A2, A3, A4};

// --- NASTAWY PID ---
// float Kp = 0.15;
// float Ki = 0.001;
// float Kd = 0.8;

// jacob - tymczasowe NASTAWY PID
// KP
// 0.07 - ok, TESTING
// 0.08 - ok, JEST SUPER ale troszke oscyluje
// 0.12 - ok, troche za bardzo oscyluje
// 0.20 - testing, troche oscyluje na zakretach
float Kp = 0.07;
float Ki = 0.0;
// Kd
// 0.5 - ok ale troche przestrzela
// 0.75 - testing
// 1.0 - za dużo
float Kd = 0.75;

// --- ZMIENNE POMOCNICZE ---
int blad = 0;
int poprzedni_blad = 0;
float calka = 0;

// V_BAZA ( domyslenie 180 , zasieg od 0 do 255)
// 150 - super
// 200 - super, próbuje ponownie aby miał zapas mocy na przyspieszanie
// 250 - super
int V_BAZA = 250;
int ostatni_kierunek = 0;

// zmienne pomocnicz do naprawy hamowania
unsigned long poprzedni_czas = 0; 
int korekta = 0;                  

// jacob - wydaje mi sie ze taki jest odpowieni w razie czego zmniejszyc do 800
// 850 - działa ok
// 800 - super
const int PROG_LINII = 800; // Czarna linia = niski odczyt (<400), białe tło = wysoki (>400)

void setup() {
  pinMode(m_prawy_A, OUTPUT); pinMode(m_prawy_B, OUTPUT);
  pinMode(m_lewy_A, OUTPUT); pinMode(m_lewy_B, OUTPUT);
  pinMode(en_prawy, OUTPUT); pinMode(en_lewy, OUTPUT);
  
  delay(3000); 
}

void loop() {
  int pozycja = oblicz_pozycje();

  // if (pozycja == 10000) {
  //   szukaj_linii();
  //   return;
  // }

  blad = pozycja;
  // test start
  // Pobieramy aktualny czas w milisekundach
  unsigned long aktualny_czas = millis();

  // Liczymy PID tylko jeśli minęło 5 milisekund (czyli 200 razy na sekundę)
  if (aktualny_czas - poprzedni_czas >= 5) {
      calka = calka + blad;
      calka = constrain(calka, -3000, 3000);

      int rozniczka = blad - poprzedni_blad;
      
      korekta = (Kp * blad) + (Ki * calka) + (Kd * rozniczka);
      
      poprzedni_blad = blad;
      poprzedni_czas = aktualny_czas; // Zapisujemy czas do następnego sprawdzenia
  }
  // test end

  // === NOWY MODUŁ DYNAMICZNEJ PRĘDKOŚCI ===
  // Twój współczynnik hamowania. 
  // 0.0 = wyłączone, robot jedzie ciągle 250
  // 0.5 = średnie hamowanie
  // 1.0 = ostre hamowanie przed każdym łukiem
  float wspolczynnik = 0.5; 
  
  // Odejmujemy część korekty od prędkości maksymalnej.
  // Używamy abs(korekta), bo chcemy zwalniać niezależnie czy skręcamy w lewo czy w prawo.
  int aktualne_v = V_BAZA - (abs(korekta) * wspolczynnik);

  // Zabezpieczenie krytyczne! 
  // Nie pozwalamy, aby na ekstremalnie ostrym zakręcie baza spadła poniżej pewnego progu (np. 80), 
  // bo robot całkowicie by się zatrzymał.
  if (aktualne_v < 80) {
      aktualne_v = 80;
  }

  // Obliczamy ostateczną moc na koła używając nowej, bezpiecznej bazy
  int moc_lewy  = aktualne_v + korekta;
  int moc_prawy = aktualne_v - korekta;

  move(moc_lewy, moc_prawy);
}

int oblicz_pozycje() {
  long suma_wag = 0;
  int aktywnych = 0;
  int wagi[] = {-2000, -1000, 0, 1000, 2000};
  
  for (int i = 0; i < 5; i++) {
    int odczyt = analogRead(czujniki[i]);
    if (odczyt < PROG_LINII) {
      suma_wag += wagi[i];
      aktywnych++;
    }
  }

  // if (aktywnych == 0) return 10000;
  if (aktywnych == 0) {
    // Zamiast zwracać 10000, dajemy maksymalny możliwy błąd z "plusem" lub "minusem",
    // aby PID sam agresywnie, ale płynnie skręcił i użył dynamicznego hamowania.
    if (ostatni_kierunek == 1) return 3000; 
    else return -3000;
  }

  int pozycja = suma_wag / aktywnych;

// old
  // if (pozycja < 0) ostatni_kierunek = -1;
  // else if (pozycja > 0) ostatni_kierunek = 1;

// new
  // Zapisujemy kierunek TYLKO jeśli linia jest wyraźnie po jednej ze stron.
  // Ignorujemy małe wahania wokół zera (środka).
  if (pozycja < -500) {
      ostatni_kierunek = -1; // Linia jest zdecydowanie po lewej
  } 
  else if (pozycja > 500) {
      ostatni_kierunek = 1;  // Linia jest zdecydowanie po prawej
  }

  return pozycja;
}

// void szukaj_linii() {
//   if (ostatni_kierunek == -1) move(-80, 80); 
//   else move(80, -80);
// }

void move(int ml, int mp) {
  ml = constrain(ml, -255, 255);
  mp = constrain(mp, -255, 255);

  if (ml >= 0) {
    digitalWrite(m_lewy_A, HIGH); digitalWrite(m_lewy_B, LOW);
  } else {
    digitalWrite(m_lewy_A, LOW); digitalWrite(m_lewy_B, HIGH);
    ml = -ml;
  }

  if (mp >= 0) {
    digitalWrite(m_prawy_A, HIGH); digitalWrite(m_prawy_B, LOW);
  } else {
    digitalWrite(m_prawy_A, LOW); digitalWrite(m_prawy_B, HIGH);
    mp = -mp;
  }

  analogWrite(en_lewy, ml);
  analogWrite(en_prawy, mp);
}
