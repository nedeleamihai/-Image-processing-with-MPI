Mai intai vom adauga toate filtrele intr-un vector.
Vom citi din fisierul de intrare culoarea(P5 sau P6), lungimea, latimea imaginii si bufferul de pixeli.
Buferul cu pixeli va fi alocat cu LungimeImagine = width * height * numcolors.
Numcolors initial este 1. Daca imaginea este color numcolors se va face 3.
Vom verifica pentru fiecare proces cate linii trebuie sa citeasca acesta din matrice.

In procesul 0 vom trimite LiniiProcese[rank] linii la fiecare proces ca sa se aplice filtrele pe acestea.
De asemenea, procesul 0 va aplica filtrele pe LiniiProcese[0] linii.
Vom folosi functia AplicaFiltre pentru a aplica filtrele pe linii.

La final, in procesul 0 va primi de la toate procesele liniile modificate si va scrie noua matrice in imaginea output.
