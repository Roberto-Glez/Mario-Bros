# 🏗️ Super Maistrín Bros. - Obra Negra Edition

> Un videojuego de plataformas en C++ inspirado en Super Mario Bros, en el que eres un chalán que debe superar peligrosas obras negras para asegurar su pago de la semana. El chalán evoluciona a través de los niveles, adquiriendo poderes para superar los desafíos, convirtiendose en Maistro para terminar la obr.
---

## 🎯 Objetivo

El objetivo principal del juego es guiar al **Maistrín** (un trabajador de la construcción) a través de peligrosas obras negras inconclusas.

El jugador debe superar obstáculos plataformeros, evitar materiales de construcción que han cobrado vida y utilizar bebidas y comidas de manera estratégica para llegar al final del trayecto y asegurar su pago de la semana "la raya". El reto consiste en superar el **Nivel 1 (La Estructura)** y prepararse para el **Nivel 2 (Parkour de Andamios)**.

---

## 🎮 Controles

El juego utiliza el teclado para controlar al personaje.

- [→]. Correr a la derecha
- [←]. Correr a la izquierda
- [↑]. Saltar
- [↓]. Agacharse (El Maistro/El Maistro Fiestero)
- [Espacio]. Lanzar caguama (El Maistro Fiestero)

---

## ⚙️ Mecánicas del Juego

El proyecto es un plataformero 2D basado en físicas (usando **Box2D**), lo que permite interacciones realistas en saltos, colisiones y movimiento de objetos.

### 1. Sistema de Evolución Laboral (Power-ups)
El jugador progresa a través de una jerarquía de estados basada en los items que recolecta, estos de obtienen al saltar por debajo de un **Plano de pregunta** y agarrar el objeto.

* **👷 El Chalán (Estado Pequeño):** El estado inicial. Es ágil pero frágil; un solo golpe de cualquier enemigo lo elimina.

* **🧱 El Maistro (Estado Grande):** Al consumir una **Coca de vidrio**, el Chalán evoluciona. Gana altura y puede resistir un golpe adicional antes de volver a ser Chalán.

* **🍺 El Maistro Fiestero (Estado de Poder):** Al encontrar una **Torta de jamón**, el Maistro alcanza su máximo potencial. Obtiene la habilidad de lanzar caguamas como proyectiles para eliminar enemigos a distancia.

### 2. Enemigos
Los peligros de la obra tienen comportamientos únicos:

* **Sacos de Cemento:** Enemigos básicos que patrullan las plataformas. Para derrotarlos hay que saltar sobre ellos y aplastarlos.

* **Tanques de Gas:** Al saltar sobre ellos, se tumban como mecanismo de defensa y rodar para hacerte daño. Si se patean, ruedan a alta velocidad, rebotando en paredes y haciendo daño. Para derrotarlos completamente, es necesario la habilidad de El Maistro Fiestero para lanzarle una caguama.

### 3. Riesgos Ambientales (Insta-kill)

* **La Tabla con Clavos Oxidados:** La trampa más peligrosa. A diferencia de los enemigos normales, tocar esta trampa causa tétanos, eliminando al jugador inmediatamente sin importar si está en estado Grande o Fiestero. Requiere saltos precisos para evitarla.

### 4. Arnés de sguridad

* **Pegarse a las paredes:** EL maestrín tiene un arnés de seguridad que lo hace pegarse a las paredes, no las puede escalar pero le da tiempo a reflexionar que puede hacer en la situación en la que está.

---

## 👥 Créditos

**Equipo de Desarrollo":**
* **Roberto Lázaro González Espinoza**
* **Alan Enrique Avelar Lamadrid**

**Tecnologías Utilizadas:**
* **Lenguaje:** C++
* **Motor Gráfico/Audio:** SFML 3.0 (Simple and Fast Multimedia Library)
* **Motor de Físicas:** Box2D 3.1
* **Herramientas:** Piskel (Arte).

**Sprites:**
Wiktor: Plataformas https://mfgg.net/index.php?act=resdb&param=02&c=1&id=41834

BullyWithAHat: Bloques de entorno (Tilesets) https://mfgg.net/index.php?act=resdb&param=02&c=1&id=41798

**Sonidos:**
WobbleBoxx Workshop: Meta https://opengameart.org/content/level-up-power-up-coin-get-13-sounds

Lokif: Sonidos de interfaz y ambiente https://opengameart.org/content/gui-sound-effects

ViRiX Dreamcore: Power-up https://opengameart.org/content/ui-and-item-sound-effect-jingles-sample-2

Crystal Games: Salto https://opengameart.org/content/jumping-man-sounds

MentalSanityOff: Sonido de aplastar https://opengameart.org/content/jump-landing-sound

---
*Proyecto educativo sin fines de lucro. Las referencias a marcas son con fines de parodia.*