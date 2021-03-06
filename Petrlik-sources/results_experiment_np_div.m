%close all
clear all

%Change to directory containing results
dir_path = 'output/result/exp_np_div/';

iterations = 100;


index = 1;
for div=1:5
    not_reached_count = 0;
    results_mean(index,1).goal_reached = 0;
    sum.path_length(index,1) = 0;
    sum.iterations(index,1) = 0;
    sum.runtime(index,1) = 0;
    for iter=1:iterations
        load(strcat(dir_path, 'div', int2str(div), 'iter', int2str(iter), '.mat'));
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

figure_name = sprintf('Narrow passage divisor experiment');

%% Plotting
figure('Name', figure_name, 'NumberTitle','off');
subplot(2,2,1);
bar([results_mean(:,1).iterations]);
xlabel('Divisor');
ylabel('Iterations');
title('Iterations');

subplot(2,2,3);
bar([results_mean(:,1).path_length]);
xlabel('Divisor');
ylabel('Path length');
title('Path length');

subplot(2,2,2);
bar([results_mean(:,1).runtime]);
xlabel('Divisor');
ylabel('Runtime');
title('Runtime');

subplot(2,2,4);
bar([results_mean(:,1).goal_reached]);
xlabel('Divisor');
ylabel('Goal reached');
title('Goal reached');
