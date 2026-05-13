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
float Kp = 0.05;
float Ki = 0.0;
float Kd = 0.5;

// --- ZMIENNE POMOCNICZE ---
int blad = 0;
int poprzedni_blad = 0;
float calka = 0;

// jacob - do testow zmniejszam do 70 ( domyslenie 180 , zasieg od 0 do 255)
// 70 - ok
// 150 - super
// 200 - super
// 250 - testing
int V_BAZA = 250;
int ostatni_kierunek = 0;

// zmienne pomocnicz do naprawy hamowania
unsigned long poprzedni_czas = 0; 
int korekta = 0;                  

// jacob - wydaje mi sie ze taki jest odpowieni w razie czego zmniejszyc do 800
// 850 - działa ok
// 800 - testuje
const int PROG_LINII = 800; // Czarna linia = niski odczyt (<400), białe tło = wysoki (>400)

void setup() {
  pinMode(m_prawy_A, OUTPUT); pinMode(m_prawy_B, OUTPUT);
  pinMode(m_lewy_A, OUTPUT); pinMode(m_lewy_B, OUTPUT);
  pinMode(en_prawy, OUTPUT); pinMode(en_lewy, OUTPUT);
  
  delay(3000); 
}

void loop() {
  int pozycja = oblicz_pozycje();

  if (pozycja == 10000) {
    szukaj_linii();
    return;
  }

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
  
  // calka = calka + blad;
  // calka = constrain(calka, -3000, 3000);

  // int rozniczka = blad - poprzedni_blad;
  // int korekta = Kp * blad + Ki * calka + Kd * rozniczka;
  // poprzedni_blad = blad;

  int moc_lewy  = V_BAZA + korekta;
  int moc_prawy = V_BAZA - korekta;

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

  if (aktywnych == 0) return 10000;

  int pozycja = suma_wag / aktywnych;

  if (pozycja < 0) ostatni_kierunek = -1;
  else if (pozycja > 0) ostatni_kierunek = 1;

  return pozycja;
}

void szukaj_linii() {
  if (ostatni_kierunek == -1) move(-80, 80); 
  else move(80, -80);
}

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
