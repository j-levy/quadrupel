close all;
importdata('data.mat')
fig=figure();

subplot(211);

plot(roll.Motor1);

hold on;
plot(roll.Motor2);
hold on;
plot(roll.Motor3);
hold on;
plot(roll.Motor4);
xlabel('Time (1/10th s)') % x-axis label
ylabel('Motor speed (RPM)') % y-axis label
legend('Motor 1','Motor 2', 'Motor 3', 'Motor 4')
title('Evolution of motor speeds for roll control')


subplot(212);
plot(roll.PSI);
hold on;
plot(roll.SP);
xlabel('Time (1/10th s)') % x-axis label
ylabel('Sensor values (arbitrary scale -32768 to 32767)') % y-axis label
legend('Angle PHI','Rate SP')
title('Evolution of sensor values for roll control')

set(findall(fig, 'Type', 'Line'),'LineWidth',2);
