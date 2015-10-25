function [ guiding_paths ] = get_guiding_path(  )
%GET_GUIDING_PATH Summary of this function goes here
%   Detailed explanation goes here

global params goals

for m=1:length(goals)
    map_grid = get_map_grid(m); % cel� �tverec se spo��t� do jeho lev�ho horn�ho (nejsem si jist, prov��it) bodu, t�m p�dem se mapa "posune"
    % toto "posunut�" zp�sobuje probl�my, pokud je p�ek�ka o 1
    % blok "zv�t�ena", t�m se zv�t�� pouze vlevo, ale nikoli vpravo
            
    % prom�nn� v matici map_grid: 
    % 3 je bu�ka s c�lem
    % 2 je bu�ka s UAV
    % 1 je bu�ka s p�ek�kou
    % 0 ke pr�zdn� bu�ka

    if params.grid_to_nodes_old
        grid_nodes = grid_to_nodes_old( map_grid );
    else
        grid_nodes = grid_to_nodes( map_grid );
    end
    [i, j] =  find(map_grid == 2, 1); % node s UAV nejv�ce vlevo dole (2. parametr u metody find je 1)
    p1 = i*size(map_grid,2)+j;  % chyba s posunut�m o 1 ��dek v��e, matlab indexuje od 1 a tak by m�l od i ode��st 1, aby bylo id spr�vn�
    [i, j] =  find(map_grid == 3, 1); % node s c�lem nejv�ce vlevo dole (2. parametr u metody find je 1)
    p2 = i*size(map_grid,2)+j;  % chyba s posunut�m o 1 ��dek v��e, matlab indexuje od 1 a tak by m�l od i ode��st 1, aby bylo id spr�vn�
    guiding_path = a_star(p1, p2, grid_nodes);

    % p�idal jsem
    assignin('base', 'p1', p1);
    assignin('base', 'p2', p2);
    assignin('base', 'grid_nodes', grid_nodes);
    assignin('base', 'guiding_path', guiding_path);
    assignin('base', 'map_grid', map_grid);
    % konec p�id�n�
    
    guiding_paths{m} = guiding_path; %#ok
    if params.algorithm==2 && params.final_path
        plot([guiding_path.x], [guiding_path.y], 'color', [170 222 135]./255, 'linewidth', 2)
        hold on
        drawnow
    end
end

end

