# Optimizacion-Estacion-Gases-educaCont
Firmware de la Estacion Calidad Aire EducaCont Corregido por INOPYA

  Correccion del firmware de la Estacion de medicion de calidad del aire  del Proyecto EducaCont 
  para la optimizacion de  uso de memoria SRAM y solventar asi la imposibilidad de añadir nuevos sensores 
  y el aprovechamiento del kit de expansion que se puede adquirir para dicha estacion.
  
  Puede consultarse el codigo original (con sus defectillos) en:
  * https://sites.google.com/view/educacont/documentaci%C3%B3n/firmwares
  * https://drive.google.com/file/d/1D3JPS6QPwnk5GonNRx_Yl9s7XHZ3LTxn/view?usp=drive_web
  Tambien podeis econtrar una copia de dicho firmware sin modificaciones en este repositorio.

  Se ha optimizado el codigo para corregir dichos problemas derivados de la mala gestion de memoria 
  y añadido el sensor de presion BMP180 a modo de ejemplo.
  
  Con solo usar la macro F() para el compilador, liberamos 671 bytes en cadenas de texto.
  Podemos asi mismo, aligerar tambien el uso de memoria Flash si condicionamos dichas cadenas de texto con
  #ifdef / #endif, ya que si bien pueden tener alguna utilidad en modo 'DEBUG', 
  son totalmente innecesarias en el modo de operacion normal.
  Se ha reestructurado el codigo y sus comentarios para una mayor legibilidad.
  
  * Recordemos siempre que el codigo se escribe una vez, pero posiblemente se lea muchas veces y/o por muchos.
