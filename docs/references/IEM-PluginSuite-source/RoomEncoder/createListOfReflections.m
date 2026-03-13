xRange = -3:3;
yRange = -3:3;
zRange = -2:2;

maxOrder = 7;

cmbs = allcomb(xRange,yRange,zRange);
order = sum(abs(cmbs),2);
[order,idx] = sort(order);
cmbs = cmbs(idx,:);

sum (order <= maxOrder)
refl = [cmbs(order<=maxOrder,:) order(order<=maxOrder)];
for o = 1 : maxOrder
    fprintf ('%i ', find(refl(:, 4) == o, 1) - 1)
end
%%
mat2header('reflections',refl,1);