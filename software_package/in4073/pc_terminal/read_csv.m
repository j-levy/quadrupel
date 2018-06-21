clear all;
close all;
filename = 'filtered_data.csv';
finished = false;
set(gcf,'CurrentCharacter','@'); % set to a dummy character

if (~exist('startrow')) 
    startrow = 0;
end
MaxNbrOfPoints = 700;
counter = 0;

M = csvread(filename, startrow, 0);
time = [];
raw = [];
filt = [];

time_read = M(:,1);
raw_read = M(:,2);
filt_read = M(:,3);


time = [time,time_read(:) ];
startrow = length(time)-1;
time = [];

fig = figure;
h = animatedline('MaximumNumPoints',MaxNbrOfPoints, 'Color', 'b');
j = animatedline('MaximumNumPoints',MaxNbrOfPoints, 'Color', 'r');
while ~finished
    M = csvread(filename, startrow, 0);
    time = [];
    raw = [];
    filt = [];

    time_read = M(:,1);
    raw_read = M(:,2);
    filt_read = M(:,3);


    time = [time,time_read(:) ];
    raw = [raw, raw_read(:)];
    filt = [filt, filt_read(:)];
    startrow = startrow + length(time) - 1;
    
    pause (0.05);
    
    %{
    animatedline (time, raw, 'Color','b');
    animatedline (time, filt, 'Color','r');
    %}
    
    for k = 1:length(time)
        addpoints(h,time(k),raw(k));
        addpoints(j,time(k),filt(k));
        drawnow limitrate
    end
    %{
    if (counter >= MaxNbrOfPoints)
        clf(fig)
        counter = 0;
        time = [];
        raw = [];
        filt = [];
    else
        counter = counter + 1
    end
    %}
end
    
