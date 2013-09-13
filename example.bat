SET download=X:\download
mkdir extract

mkdir extract\image\adv
mapngdecoder %download%\image\adv\*
move %download%\image\adv\*.png extract\image\adv

mkdir extract\image\boss
mapngdecoder %download%\image\boss\*
move %download%\image\boss\*.png extract\image\boss

mkdir extract\image\card
mapngdecoder %download%\image\card\*
move %download%\image\card\*.png extract\image\card

mkdir extract\image\face
mapngdecoder %download%\image\face\*
move %download%\image\face\*.png extract\image\face

mkdir extract\image\gacha
mapngdecoder %download%\image\gacha\*
move %download%\image\gacha\*.png extract\image\gacha