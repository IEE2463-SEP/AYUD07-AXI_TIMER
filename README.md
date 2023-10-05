# AYUD07-AXI_TIMER

En esta ayudantía utilizaremos el IPCore AXI Timer en nuestro diseño de hardware para luego habilitar interrupciones periódicas mediante software. Se implementa en la tarjeta ZYBOZ7.

Ejercitaremos el uso de interrupciones y también aprenderemos a habilitar el modo PWM que admite AXI Timer, esto último para variar la intensidad del canal R del led RGB modificando el duty cycle de la señal PWM que generaremos.

Adicionalmente también revisaremos cómo programar la Zybo desde ambos programas: desde Vitis y desde Vivado.

1. Previo a nuestra ayudantía debes revisar este [video]() y ejecutarlo en tu casa.

2. Los documentos asociados a la ejecución de este laboratorio son:
    * [AYUD07-Guia Ejercicio]():  Guía de ejercicios asociado a la ayudantía. 
    * [Timer_Ints.c](): El cual contiene el código C utilizado en la ayudantía para generar las interrupciones
    * [Zybo-Z7_Master.xdc]():  El cual contiene los constraints de nuestra tarjeta (mapeo de pines)    
    * [Ay_AXI_Timer_Ints.zip]():  Contine el projecto en vivado desarrollado en el laboratorio    
   
3. Durante la ayudantía se busca que desarrollen el ejercicio propuesto de la guía.
