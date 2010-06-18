% MATLAB/Octave script to convert redshift gamma values to flux values
gam_rs = csvread('gammars.csv');
ratio=(6500-3400)/(6500-4700);
temps=1000:100:10000;
temps_flux=(temps-6500)*ratio + 6500;
gam_new=interp1(temps_flux,gam_rs,temps);

fid=fopen('newgamma.csv','w');
for i=1:length(gam_new)
	fprintf(fid,'\t%.4ff, %.4ff, %.4ff, /* %d */\n',...
		gam_new(i,1),gam_new(i,2),gam_new(i,3),temps(i));
end
fclose(fid);

