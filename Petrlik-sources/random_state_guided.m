function [ s_rand ] = random_state_guided( guiding_paths, index, goal_reached )
%RANDOM_STATE_GUIDED Summary of this function goes here
%   Detailed explanation goes here
% v prom�nn� index je matice, sloupec pro cestu, ��dek pro UAV. ��k�, kolik
% nodes zb�v� UAV ne� doraz� na konec cesty

global number_of_uavs params

sum = 0; % sum = celkov� cesta v�ech vedouc�ch cest
for m=1:length(guiding_paths)
    l = length(guiding_paths{m}); % ��m del�� cesta, t�m v�t�� pravd�podobnost, �e se tou cestou vyd�
    probabilities(1,m) = l; %#ok propabilities je norm�ln�, 1D pole. nech�pu, pro� tady pou��v� notaci pro 2D pole
    sum = sum + l;
end

probabilities = probabilities./sum;
s_rand = NaN(2,number_of_uavs);

%Random path sampled for each uav
if params.rrt_path_sampling_case == 1
    for n=1:number_of_uavs
        rn2 = rand();   % ur�uje, zda se vybere n�hodn� bod nebo bod na vedouc� cest�
        if rn2 > params.guided_sampling_prob
            s_rand(:,n) = random_state(0, params.world_dimensions(1,1), ...
                0, params.world_dimensions(2,1));
        else                % bod na vedouc� cest�
            rn = rand();
            add = 0;
            for m = 1:length(probabilities) % length(propabilities) je po�et vedouc�ch cest
                if rn < probabilities(m) + add
                    guiding_path = guiding_paths{m};
                    center = [guiding_path(index(n,m)).x; ...
                        guiding_path(index(n,m)).y];
                    if goal_reached(1,n) == m
                        s_rand(:,n) = random_state_goal(goal_reached(1,n));
                    else
                        while true
                            s_rand(:,n) = random_state_polar(center, 0, params.sampling_radius);
%                            if ~check_inside_obstacle(s_rand(:,n))
                                break
%                            end
                        end
                        break
                    end
                else
                    add = add + probabilities(m);
                end
            end
        end
    end
    
    %Same path sampled for each uav
elseif params.rrt_path_sampling_case == 2
    rn2 = rand();
    if rn2 > params.guided_sampling_prob
        for n=1:number_of_uavs
            s_rand(:,n) = random_state(0, params.world_dimensions(1,1), ...
                0, params.world_dimensions(2,1));
        end
    else
        rn = rand();
        add = 0;
        for m=1:length(probabilities)
            
            if rn < probabilities(m) + add
                for n=1:number_of_uavs
                    guiding_path = guiding_paths{m};
                    center = [guiding_path(index(n,m)).x; ...
                        guiding_path(index(n,m)).y];
                    if goal_reached(1,n) == m
                        s_rand(:,n) = random_state_goal(goal_reached(1,n));
                    else
                        while true
                            s_rand(:,n) = random_state_polar(center, 0, params.sampling_radius);
                            if ~check_inside_obstacle(s_rand(:,n))
                                break
                            end
                        end
                        break
                    end
                end
                break
            else
                add = add + probabilities(m);
            end
        end
    end
    
    %Every UAV has its guiding path
elseif params.rrt_path_sampling_case == 3
    rn2 = rand();
    if rn2 > params.guided_sampling_prob    % vyb�r� se n�hodn� vzorek
        for n=1:number_of_uavs
            if goal_reached(1,n) == m        % pokud je n-t� UAV v c�li, vybere se n�hodn� bod z c�lov� plochy, kam UAV dorazilo
                s_rand(:,n) = random_state_goal(goal_reached(1,n));
            else                             % pokud n-t� UAV nen� v c�li, vybere se n�hodn� bod z cel� mapy
                s_rand(:,n) = random_state(0, params.world_dimensions(1,1), ...
                    0, params.world_dimensions(2,1));
            end
        end
    else
        ratio = number_of_uavs/length(guiding_paths); % pom�r UAV k po�tu vedouc�ch cest
        for m=1:length(guiding_paths)
            guiding_path = guiding_paths{m};
            best_reached = min(index((m-1)*ratio+1:(m-1)*ratio+ratio,m));
            
%             assignin('base', 'index', index((m-1)*ratio+1:(m-1)*ratio+ratio,m));
%             assignin('base', 'best_reached', best_reached);
            
            center = [guiding_path(best_reached).x; ...
                guiding_path(best_reached).y];
            for n=1:ratio
                if goal_reached(1,(m-1)*ratio+n) == m
                    s_rand(:,(m-1)*ratio+n) = random_state_goal(goal_reached(1,(m-1)*ratio+n));
                else    % n�hodn� pozice z okruhu okolo nejbli���ho bodu nejbli��� vedouc� cesty
                    while true
                        s_rand(:,(m-1)*ratio+n) = random_state_polar(center, 0, params.sampling_radius);
   %                     if ~check_inside_obstacle(s_rand(:,(m-1)*ratio+n))
                            break
    %s                    end
                    end
   %                 break
                end
            end
        end
    end
else
    fprintf('%d is invalid Guided path sampling case.', ...
        params.rrt_path_sampling_case);
end
end

