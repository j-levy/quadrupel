close all;
importdata('data.mat')
fig=figure();

subplot(211);

plot(yaw.Motor1);

hold on;
plot(yaw.Motor2);
hold on;
plot(yaw.Motor3);
hold on;
plot(yaw.Motor4);
xlabel('Time (1/10th s)') % x-axis label
ylabel('Motor speed (RPM)') % y-axis label
legend('Motor 1','Motor 2', 'Motor 3', 'Motor 4')
title('Evolution of motor speeds for yaw control')


subplot(212);
plot(yaw.SR);
xlabel('Time (1/10th s)') % x-axis label
ylabel('Sensor values (arbitrary scale -32768 to 32767)') % y-axis label
legend('Rate Sr')
title('Evolution of sensor values for yaw control')

set(findall(fig, 'Type', 'Line'),'LineWidth',2);
