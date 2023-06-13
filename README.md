# -Air_quality_control_station
Kod napisany pod Arduino Uno, pisany w środowisku Visual Studio Code z pomocą rozszerzenia PlatformIO, który monitoruje jakość powietrza przy użyciu czujnika temperatury i wilgotności DHT11 oraz czujnika pyłu. Wyświetla temperaturę i wilgotność na wyświetlaczu LCD dodatkowo jakość powietrza (mierzoną przez czujnik pyłu) sygnalizowana jest za pomocą diod LED RGB. Kod zawiera również przerwanie przycisku, aby przełączać wyświetlanie temperatury/wilgotności i jakości powietrza na LCD.

Oto krótki opis struktury kodu:

Biblioteki i definicje pinów: Zostały dołączone niezbędne biblioteki, a także zostały zdefiniowane piny dla czujników, diod LED i wyświetlacza LCD.

Zmienne globalne: Zadeklarowane są zmienne do przechowywania poprzednich odczytów czujników, flagi statusu wyświetlacza LCD oraz aktualnego poziomu jakości powietrza.

Funkcje: Zdefiniowano kilka pomocniczych funkcji do różnych zadań, takich jak ustawianie koloru diod LED RGB na podstawie jakości powietrza, przewijanie tekstu na wyświetlaczu LCD, drukowanie wartości pyłu, czy czyszczenie wyświetlacza.

Inicjalizacja: W funkcji setup() inicjalizowane są niezbędne elementy, takie jak uruchomienie komunikacji szeregowej, inicjalizacja czujników, wyświetlacza LCD oraz przycisku.

Pętla główna: W funkcji loop() następuje główna pętla programu. Jeśli flaga isDustOnTop jest ustawiona, wywoływana jest funkcja DustOnTop() do monitorowania jakości powietrza na górnym wierszu LCD. W przeciwnym razie wywoływana jest funkcja TempOnTop() do monitorowania temperatury i wilgotności na górnym wierszu LCD.
