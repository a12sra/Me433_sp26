%% Detent / Click Effect
clear; clc; close all;

x = linspace(-1,1,1000);

F = 2*x - 5*x.*exp(-(x/0.15).^2);

figure;
plot(x,F,'LineWidth',3);
grid on;
xlabel('Displacement');
ylabel('Force');
title('Haptic Detent');

saveas(gcf,'detent_curve.png');