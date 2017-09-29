Documentation "rxData" v 1.0
==
[TOC]

Une librairie réactive **c++** minimaliste.

---
Sommaire
--

#### 1. Affectation de valeurs
```
rxData<float> a, b, c;

c = (a + b) * a;
a = 10;
b = 42;
std::cout << c << std::endl;
a = 1;
std::cout << c << std::endl;
```
```
>g++ main.cpp -std=c++11 && ./a.out
520
43
```
> Lors d'opérations il ne faut pas mélanger des données réactives avec des données standard. Car dans ce cas les données réactives dans l'opération ne seront pas considérées comme étant réactive. Si vous souhaitez faire cela, vous pouvez utiliser la méthode subscribe ou encore, merge2, merge3 ou merge 4.

---
#### 2. Abonnement à des données.
```
rxData<float> a, b, c, d;
a = 2;
b = 4;
c = 10;
d = a + c * b; // => d:42 (2 + 4 * 10)
d.observe(1, [](float value) {
	std::cout << "d:" << value << std::endl;
});
a = 3; // => d:43 (3 + 4 * 10)
b = 2; // => d:23 (3 + 2 * 103 + 2 * 103 + 2 * 10)
d = 789; // => d:789
d = a + b + c; // => d:(3 + 2 + 10)
d.unobserve(1); // L'observer a l' id 1 ne sera plus notifié
a = 1000; // ne fait rien;
```
```
>g++ main.cpp -std=c++11 && ./a.out
d:42
d:43
d:23
d:789
d:15
```
> - L'abonnement passe par la méthode `void observe(int key, std::function<void (T)> command)`.
> - Le premier argument, un entier qui permettra de se désabonner grâce à la méthode `void unobserve(int id)`.
> - Le deuxième argument est une fonction lambda qui recevra en argument la valeur. Celle-ci sera automatiquement appelée lors de l'abonnement, puis à chaque fois que sa valeur sera changée.

---
#### 3. Mélange de données
```
 rxData<float> f;
 rxData<int> i;
 rxData<double> d;

 f = 3.14;
 i = 42;
 d = rxData<double>::merge2<float, int>(f, i, [](float fValue, int iValue) {
	 return ((iValue * 4) + fValue);
 });
 d.observe(1, [](double value) {
	 std::cout << value << std::endl;
 });
 while (i > 0)
	 i = i - 1;
``` 
```
>g++ main.cpp -std=c++11 && ./a.out
171.14
167.14
...
11.14
7.14
3.14
```
> Les méthodes merge2, merge3 et merge4 fonctionnent de la même  manière.
> Si une donnée réactive doit dépendre de plusieurs autres données réactives, ces méthodes sont bien pratiques.

---
#### 4. Opération un peu plus compliqué que les opérateurs de base
```
rxData<float> a, b;

a = 16;
a.subscribe(&b, [&b](float value) {
	b.notify(sqrt(value));
});
b.observe(1, [](float value) {
	std::cout << value << std::endl;
});
a = 9;
a = 4;
a = 1;
return 0;
```
```
>g++ main.cpp -std=c++11 && ./a.out
0
3
2
1
```
> Pour des opérations mathématiques autres que celles de bases comme +-*/ on peut utiliser utiliser`void subscribe(void *who, std::function<void (T)> command)` pour s'abonner à une donnée réactive et `void notify(T value)` pour mettre a jours la donnée dépendante.
> C'est ce que fait implicitement merge2, 3 et 4 ou encore les opérations comme (+, -, *, /).

---

#### 5. cast
```
rxData<float> a;
rxData<int> b;

a = 0.1;
b = a.rxCast<int>();
b.observe(1, [](int value) {
	std::cout << value << std::endl;
});
while (a <= 3)
	a = a + 0.1;
```
```
>g++ main.cpp -std=c++11 && ./a.out
0
1
2
3
```

> Pour caster une donné dans un autre type de donnée :`rxData<U> &rxCast()`
> À noter que la variable "b" n'est notifiée que si sa valeur change et non pas a chaque fois qu'une de ses dépendances est notifiée

#### 6. printDependance
```
rxData<float> a, b, c, d, e, f;

a = 6;
b = 7;
c = rxData<float>::merge2<float, float>(d, e, [](float dValue, float eValue) {
	return (dValue + eValue / 2);
});
d = 3;
e = 5;
f = c + b * a;
std::cout << "id a:" << a.getId() << "[" << a << "]" << std::endl;
std::cout << "id b:" << b.getId() << "[" << b << "]" << std::endl;
std::cout << "id c:" << c.getId() << "[" << c << "]" << std::endl;
std::cout << "id d:" << d.getId() << "[" << d << "]" << std::endl;
std::cout << "id e:" << e.getId() << "[" << e << "]" << std::endl;
std::cout << "id f:" << f.getId() << "[" << f << "]" << std::endl;
std::cout << "==========printDependancesReverse===========" << std::endl;
f.printDependancesReverse(0);
std::cout << "==============printDependances==============" << std::endl;
a.printDependances(0);
```
```
>g++ main.cpp -std=c++11 && ./a.out
id a:1[6]
id b:2[7]
id c:3[5.5]
id d:4[3]
id e:5[5]
id f:6[47.5]
==========printDependancesReverse===========
6[47.5]
	9[47.5] +
		3[5.5]
			7[5.5] merge2
				4[3]
				5[5]
		8[42] *
			2[7]
			1[6]
==============printDependances==============
1[6]
	8[42] *
		9[47.5] +
			6[47.5]
```

> La méthode printDependancesReverse permet d'afficher toutes les variables réactives dont dépend une donnée réactive.
> La méthode printDependances permet d'afficher toutes les variables réactives dépendante d'une variable.
