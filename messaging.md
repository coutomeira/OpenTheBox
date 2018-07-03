# Messaging protocols between devices

## Arduino -> RaspberryPi

* Notas:

  Analisar apenas mensagens "PI:"

  p.ex "PI:TS:OK\n"      ou "PI:TS:NOK\n"

  p.ex "PI:ARDUINO:1:OK\n" ou "PI:ARDUINO:2:OK\n"

  p.ex "PI:MSG:NOK:[BADMSG,BADSTART,NOTEND]" *UM DOS TRÊS, CASO NÃO RECEBAS NADA SOBRE MSG É PQ A MSG FOI BOA*

** ARDUINO ONE (TFT)

* "MODE:XXX\n"

  (Depois de o utilizador escolher que método usar depois de clicar na interface do TS)

  p.ex "PI:MODE:QR\n" *INDICAÇÃO AO RAPSBERRY QUE DEVE LIGAR A CAMERA*

  p.ex "PI:MODE:PIN\n"

  p.ex "PI:MODE:MORSE\n"


* "PIN:1234\n"

  p.ex "PI:PIN:1234\n"

  (O PIN terá 4 digitos)

  (Se o número de digitos não for 4 o PIN deve ser preenchidos com zeros à esquerda)

  p.ex "PI:PIN:10\n" (Neste caso o código será 0010)


* "MORSE:BTT\nBB\nBT\nB\nBTBB\n"

  p.ex "PI:MORSE:BTT\nBB\nBT\nB\nBTBB\n"

  (B indica Bola e T indica Traço)  *O ANDROID DEVE USAR A MESMA NOTAÇÃO AO CRIAR O CÓDIGO MORSE*

  ('\n' indica novo simbolo. Serão enviados 5)

** ARDUINO TWO (MOTOR AND LED)


* LED

  p.ex "PI:LED:G\n"
  
  p.ex "PI:LED:R\n"


* MOTOR

  p.ex "PI:MOTOR:O\n"
  
  p.ex "PI:MOTOR:C\n" * Sempre que esta msg for enviada o RPI deve informar o outro Arduino 
                      * Isso é necessário para atualizar o TFT 
                      * Enviar: "AR:IMG:8X"
  
* CAIXA

  p.ex "BOX:O\n"
  
  p.ex "BOX:C\n"
  
  
  "PI:NOTPOSSIBLE" quando se manda fechar a caixa e a tampa não está fechada
  No entanto, para este trabalho não é necessário

  Quando as mensagens não são bem recebeidas seguem o protocolo do outro Arduino
  
## RaspberryPi -> Arduino

* Notas:  

  Analisar apenas mensagens "AR:"

  Todas as mensagens devem ser terminadas pelo caracter 'X' (sem \n)

  Todas as mensagens devem ser em caps lock

** ENVIO P/ ARDUINO ONE
* Notas:

  (Neste Arduino só são enviadas informação para mostrar imagens)

  (Assim sendo, todas as mensagens devem-se iniciar-se por "AR:MSG:")

  (
  Seguidamente um inteiro que da seguinte lista:

  1 -> Um código certo

  2 -> Dois códigos certos

  3 -> Três códigos certos  (p/ abrir a caixa deve ser enviada info ao Arduino One)

  4 -> Um código errado

  5 -> Dois códigos errados

  6 -> Três códigos errados (bloqueio de 60 segundos)

  7 -> Abrir a caixa imediatamente

  8 -> Fechar a caixa 
  )

  (O uso de códigos curtos permite uma leitura rápida no Arduino que é feita char a char)

  p.ex "AR:IMG:6X"

  p.ex "AR:IMG:7X"

** ENVIO P/ ARDUINO TWO

* Notas:

  (Indicação da rotação do motor)

  p.ex "AR:OPENX" 

  p.ex "AR:CLOSEX"
  
  As mensagens podem ser encurtadas para "AR:OX" e "AR:CX"

  O RPI pode questionar o Arduino sobre o atual estado da caixa 

  p.ex "AR:HOWX"  ou simplesmente  "AR:HX"
  RESPOSTAS POSSIVEIS:
  p.ex "BOX:O\n"  ou  "BOX:C\n"