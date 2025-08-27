Diferencia ante un lenguaje de alto y bajo nivel.

La principal diferencia que el bajo nivel tiene funciones como malloc que puede liberar memoria. A diferencia de python, esta tiene la garbage collector en la cual si ya no es usado ya se puede librar el espacio.

Otra diferencia que se ha encontrado es que en lenguaje de bajo nivel tiende a ser más eficientes y mejores a las secuenciales si son los mísmos hilos de la maquina que en este caso fueron 12. Por otro lado, observamos que los mejores tiempos son mejores en bajo nivel porque no tienen que overhead al crear los threads como en python. 


Tiempos con C
    En paralelo 6.499 ms
    En secuencial 41.050 ms

 Tiempos con Python
    En paralelo 150.233 ms
    En secuencial 329.295 ms

La principal diferencia de tiempo en el paralelo es más de 8 veces del tiempo transcurrido con el lenguaje de bajo nivel. Mientras que la secuencial, se tarda más de 23 veces en el secuencial. 

Para las complicaciones en bajo nivel es el manejo de espacio en la memoria ya que manualmente tenemos que distribuir. Tambien existen problemas como fugas de memoria o acceso indebido a memoria son frecuentes. Por otro lado alto nivel es más simple en programar a comparación de bajo nivel. 
