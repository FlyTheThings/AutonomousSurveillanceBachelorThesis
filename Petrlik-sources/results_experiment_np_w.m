%close all
clear all

%Change to directory containing results
dir_path = 'output/result/exp_np_w/';

iterations = 100;
%width = [10 20 30 40 50 100 150 200];
width = [10 20 30 40 50 100 150 200];

index = 1;
for map=71:78
    not_reached_count = 0;
    results_mean(index,1).goal_reached = 0;
    sum.path_length(index,1) = 0;
    sum.iterations(index,1) = 0;
    sum.runtime(index,1) = 0;
    for iter=1:iterations
        load(strcat(dir_path, 'map', int2str(map), 'iter', int2str(iter), '.mat'));
        results(index, iter) = data;
        if any(isnan(results(index,iter).output.goal_reached))
            not_reached_count = not_reached_count + 1;
            continue
        end
        results_mean(index,1).goal_reached = results_mean(index,1).goal_reached + 1;
        sum.path_length = sum.path_length + length(results(index,iter).output.path);
        sum.iterations = sum.iterations + length(results(index,iter).output.nodes);
        sum.runtime = sum.runtime + results(index,iter).output.nodes(end).time_added;
    end
    results_mean(index,1).path_length = sum.path_length(index,1)/(iterations-not_reached_count);
    results_mean(index,1).iterations = sum.iterations(index,1)/(iterations-not_reached_count);
    results_mean(index,1).runtime = sum.runtime(index,1)/(iterations-not_reached_count);
    index = index + 1;
end

figure_name = sprintf('Narrow passage width experiment');

%% Plotting
figure('Name', figure_name, 'NumberTitle','off');
subplot(2,2,1);
bar(width, [results_mean(:,1).iterations]);
xlabel('Width');
ylabel('Iterations');
title('Iterations');

subplot(2,2,3);
bar(width, [results_mean(:,1).path_length]);
xlabel('Width');
ylabel('Path length');
title('Path length');

subplot(2,2,2);
bar(width, [results_mean(:,1).runtime]);
xlabel('Width');
ylabel('Runtime');
title('Runtime');

subplot(2,2,4);
bar(width, [results_mean(:,1).goal_reached]);
xlabel('Width');
ylabel('Goal reached');
title('Goal reached');
