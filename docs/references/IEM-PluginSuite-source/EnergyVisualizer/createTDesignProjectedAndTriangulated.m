clear

% hammer aitov projection
hap = @(azi, ele) [-cos(ele).*sin(azi/2) sin(ele)] ./ (sqrt(1+cos(ele).*cos(azi/2)));

%inverse
z = @(x,y) sqrt(1-(x*2*sqrt(2)/4).^2-(y*sqrt(2)/2).^2);
ihap = @(x,y,z) [2*atan(sqrt(2)*2*-x.*z/2./(2*z.^2-1)) asin(sqrt(2)*y.*z)];
% inverse test
% azi = 0;
% ele = 0;
% 
% xy = hap(azi,ele)
% x = xy(1);
% y = xy(2);
% 
% hz = z(x,y)
% ihap(x,y,z(x,y))

%
nx = 28;
ny = 21;
[X, Y] = meshgrid(linspace(-1,1,nx), linspace(-1,1,ny));
X(1:2:end,:) = X(1:2:end,:) - 1 / nx;

X = X(:);
Y = Y(:);
figure, scatter(X,Y)


aziele = ihap(X,Y,z(X,Y));
azi = aziele(:,1);
ele = aziele(:,2);

rePro = hap(azi,ele);
xre = rePro(:,1);
yre = rePro(:,2);

dmax = 0.00001;
for ii=1:size(X,1)
    r(ii) = abs(X(ii) - xre(ii)) < dmax || abs(Y(ii) - yre(ii)) < dmax;
end

X(~r) = [];
Y(~r) = [];

hold all
scatter(X,Y)

aziele = ihap(X,Y,z(X,Y));
azi = aziele(:,1);
ele = aziele(:,2);

rePro = hap(azi,ele);
xre = rePro(:,1);
yre = rePro(:,2);





tri = delaunay(xre,yre)
triplot (tri, xre, yre, 'Color', [0.2 0.2 0.2]);

scatter (xre, yre, 300,  '.r');
xlim ([-1 1])


[x,y,z] = sph2cart(azi,ele,1);

vertices = [xre yre]';
hammerAitovSampleVertices = vertices(:);
hammerAitovSampleX = x;
hammerAitovSampleY = y;
hammerAitovSampleZ = z;

%%
str = '';
str = [str mat2header('hammerAitovSampleX',hammerAitovSampleX,false)];
str = [str mat2header('hammerAitovSampleY',hammerAitovSampleY,false)];
str = [str mat2header('hammerAitovSampleZ',hammerAitovSampleZ,false)];
str = [str mat2header('hammerAitovSampleVertices',hammerAitovSampleVertices,false)];

indices = (tri-1)';
hammerAitovSampleIndices = indices(:);
str = [str mat2header('hammerAitovSampleIndices',hammerAitovSampleIndices,true)];


fid=fopen('hammerAitovSample.h', 'wt+');
fprintf(fid, '%s', ['#ifndef hammerAitovSample_h ' newline ' #define hammerAitovSample_h ' newline newline '#define nSamplePoints ' num2str(length(hammerAitovSampleX)) newline str newline ' #endif']);
fclose(fid);
%%
X(r) = [];
Y(r) = [];
aziele = ihap(X,Y,z(X,Y));
azi = aziele(:,1);
ele = aziele(:,2);

hold all
scatter(X,Y)


%%
tDesign = getTdesign(21); % tDesign for 10th order (Spherical Harmonic Transform Toolbox)
[tDesign(:,4),tDesign(:,5)] = cart2sph(tDesign(:,1),tDesign(:,2),tDesign(:,3))

tDesignProjected = hap(tDesign(:,4),tDesign(:,5));
scatter(tDesignProjected(:,1),tDesignProjected(:,2));

%%
tri = delaunay(tDesignProjected(:,1),tDesignProjected(:,2))
triplot(tri,tDesignProjected(:,1),tDesignProjected(:,2));

%%
vertices = tDesignProjected';
vertices = vertices(:);
mat2header('tDesignProjected',vertices,0);

%%
indices = (tri-1)';
indices = indices(:);
mat2header('vertexIndex',indices,1);