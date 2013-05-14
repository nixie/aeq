figure;
load ir32.txt
load ir128.txt
load ir512.txt
load shdump512.txt


subplot(2,2,2);
[H32,W32] = freqz(ir32,1, 512, 44100); semilogx(W32, abs(H32)); axis tight;grid;
title('N=32');
subplot(2,2,3);
[H128,W128] = freqz(ir128,1, 512, 44100); semilogx(W128, abs(H128)); axis tight;grid;
title('N=128');
subplot(2,2,4);
[H512,W512] = freqz(ir512,1, 512, 44100); semilogx(W512, abs(H512)); axis tight;grid;
title('N=512');

subplot(2,2,1);
load shdump512.txt
semilogx(W512(1:257)*2, shdump512); grid; axis tight;
title('Ideal curve');

