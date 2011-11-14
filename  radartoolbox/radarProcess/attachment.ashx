clear all
close all

% Set the base URL for the GrADS Data Server (ENP fields)
baseRef = 'http://www.nomads.ncep.noaa.gov:9090/dods/wave/enp';
 
% Set the base URL for the GrADS Data Server (Global fields)
% baseRef = 'http://nomads.ncep.noaa.gov:9090/dods/wave/nww3';

% The model is run at 00Z, 06Z, 12Z and 18Z. There is ~4.33+ hour delay,
% however between when the model is run and when it is available online.
% So, the 00Z run for any day is available by 00Z + 4.33 hours.
mRuns = 0:6:18;             % model run times
mAvail = mRuns + 4.5;       % times available, use 4.5 to be conservative
utc_time = java.lang.System.currentTimeMillis;  % Current UTC in milliseconds
utc_days = utc_time / 1000 / 60 / 60 / 24;      % Convert msecs to days
tNow = utc_days + datenum(1970,1,1,0,0,0);      % UTC date/time as serial date #
tInd = find((mAvail / 24) - rem(tNow,1) <= 0,1,'last'); % most recent forecast
clear mAvail utc_time utc_days

% Put it all together to get the date and time of the most recent forecast.
dv = datevec(tNow);
dirStr = sprintf('enp%4d%02d%02d',dv(1:3));
runStr = sprintf('enp%4d%02d%02d_%02dz',dv(1:3),mRuns(tInd));

% Set the url reference
ncRef = mDataset([baseRef,'/',dirStr,'/',runStr]);

% Info about the data set can be found via: info = nj_info(ncRef);
% In this case, the variables are:
%     'dirpwsfc' primary wave direction [deg]
%     'dirswsfc' secondary wave direction [deg]
%     'htsgwsfc' significant height of combined wind waves and swell [m]
%     'perpwsfc' primary wave mean period [s]
%     'perswsfc' secondary wave mean period [s]
%     'ugrdsfc'  u-component of wind [m/s]
%     'vgrdsfc'  v-component of wind [m/s]
%     'wdirsfc'  wind direction (from which blowing) [deg]
%     'windsfc'  wind speed [m/s]
%     'wvdirsfc' direction of wind waves [deg]
%     'wvpersfc' mean period of wind waves [s]

[hsig,grd] = nj_subsetGrid(ncRef,'htsgwsfc');
pprd = nj_subsetGrid(ncRef,'perpwsfc');
pdir = nj_subsetGrid(ncRef,'dirpwsfc');

% Once we have our data, we need to close the pointer object to the GrADS
% data server.
ncRef.close(); clear ncRef

% Prep some of the elements for plotting
[X,Y] = meshgrid(grd.lon,grd.lat); % create longitude and latitude grids

model_time = datestr(grd.jdmat,30);

for i = 1:61
    Z(:,:,i) = double(reshape(hsig(i,:,:),224,373));
    Wave_Dir(:,:,i) = double(reshape(pdir(i,:,:),224,373));
end

pdir = 90 - Wave_Dir + 180;

ind = pdir > 180;
pdir(ind) = pdir(ind) - 360;
ind = pdir < -180;
pdir(ind) = pdir(ind) + 360;

u = Z.*cos(deg2rad(pdir));
v = Z.*sin(deg2rad(pdir));

dummy = repmat(nan, [224 373 61]);

for i = 1:61,

	xxxx = Z(:,:,i);
    	xxxx(isnan(xxxx))=-99999;
	dummy(:,:,i) = xxxx;
	clear xxxx
    
end

Z_netcdf = double(dummy);
clear dummy

dummy = repmat(nan, [224 373 61]);

for i = 1:61,

	xxxx = u(:,:,i);
    	xxxx(isnan(xxxx))=-99999;
	dummy(:,:,i) = xxxx;
	clear xxxx
    
end

u_netcdf = double(dummy);
clear dummy


dummy = repmat(nan, [224 373 61]);

for i = 1:61,

	xxxx = v(:,:,i);
    	xxxx(isnan(xxxx))=-99999;
	dummy(:,:,i) = xxxx;
	clear xxxx
    
end

v_netcdf = double(dummy);
clear dummy

ref_time = datenum(2000,1,1,0,0,0);
time_netcdf = grd.jdmat-ref_time;

filename = '/home/server/pi/homes/crisien/nanoos_nvs/www_nep/WWIII_Wave_Dir_Components_ENP.nc';

nc = netcdf(filename, 'clobber');                   % Create NetCDF file.

nc.title = 'U and V componets for WWIII Wave Direction for the NEP';
nc.author = 'Craig Risien (crisien@coas.oregonstate.edu)';
nc.institution = 'OSU/COAS';
nc.comments = 'This file was created using Matlab version 7.8.0.347 (R2009a). These WWIII fields are available at http://www.nomads.ncep.noaa.gov:9090/dods/wave/enp';

nc('latitude') = 224;                                 % Define dimensions.
nc('longitude') = 373;
nc('time') = 61;

nc{'latitude'} = ncfloat('latitude');
nc{'longitude'} = ncfloat('longitude');
nc{'time'} = ncdouble('time');
nc{'u_wave'} = ncfloat('latitude', 'longitude', 'time');
nc{'v_wave'} = ncfloat('latitude', 'longitude', 'time');
nc{'wave_hgt'} = ncfloat('latitude', 'longitude', 'time');

nc{'latitude'}.units = 'degrees_north';
nc{'latitude'}.long_name = 'latitude';
nc{'latitude'}.standard_name = 'latitude';
nc{'latitude'}.valid_range = [4.75, 60.5];
 
nc{'longitude'}.units = 'degrees_east';
nc{'longitude'}.long_name = 'longitude';
nc{'longitude'}.standard_name = 'longitude';
nc{'longitude'}.valid_range = [189.75, 282.75];
 
nc{'time'}.units = 'days since 2000-01-01';
nc{'time'}.long_name = 'time';
nc{'time'}.standard_name = 'time';

nc{'u_wave'}.units = 'Wave Height (meters)';
nc{'u_wave'}.fillValue_ = ncdouble(-99999);
nc{'u_wave'}.long_name = 'u_wave_dir_component';
nc{'u_wave'}.standard_name = 'u_wave_dir_component';

nc{'v_wave'}.units = 'Wave Height (meters)';
nc{'v_wave'}.fillValue_ = ncdouble(-99999);
nc{'v_wave'}.long_name = 'v_wave_dir_component';
nc{'v_wave'}.standard_name = 'v_wave_dir_component';

nc{'wave_hgt'}.units = 'Wave Height (meters)';
nc{'wave_hgt'}.fillValue_ = ncdouble(-99999);
nc{'wave_hgt'}.long_name = 'Significant Wave Height';
nc{'wave_hgt'}.standard_name = 'Significant Wave Height';


% ---------------------------- STORE THE DATA ----------------------------

nc{'latitude'}(:) = grd.lat;                        % Put all the data.
nc{'longitude'}(:) = grd.lon;
nc{'time'}(:) = time_netcdf;
nc{'u_wave'}(:) = u_netcdf;
nc{'v_wave'}(:) = v_netcdf;
nc{'wave_hgt'}(:) = Z_netcdf;

nc = close(nc);


load chelle.pallette

for i = 1:61

close all

	if i<10
		xx=strcat('0',num2str(i));
	end

	if i>=10 & i<100
		xx=strcat('',num2str(i));
	end

	if i>99
		xx=num2str(i);
	end

	FileName = strcat('nep_image_',xx,'.png');

clf

m_proj('Equidistant Cylindrical','lon',[189.75 282.75],'lat',[4.75 60.5]);
m_pcolor(X-0.125,Y-0.125,Z(:,:,i))
shading flat
caxis([0 8]);
colormap(chelle)

hold on

axis off
% specify transparent background
set(gcf,'color','none');
% create output file
set(gcf,'InvertHardCopy','off');
print(FileName,'-dpng','-r300')
cdata = imread(FileName);
imwrite(cdata, FileName, 'png', 'BitDepth', 16, 'transparency', [1 1 1]) 

end

clear i FileName

model_runtime = datestr(grd.jdmat(1),31);
model_runtime = model_runtime(:,12:end);

model_rundate = datestr(grd.jdmat(1),31);
model_rundate = model_rundate(:,1:10);

model_time = datestr(grd.jdmat,31);
model_time = model_time(:,12:end);

model_date = datestr(grd.jdmat,31);
model_date = model_date(:,1:10);

!rm /home/server/pi/homes/crisien/nanoos_nvs/www_nep/nep_wvht_overlays.json

filename = '/home/server/pi/homes/crisien/nanoos_nvs/www_nep/nep_wvht_overlays.json';
fid2 = fopen(filename,'a');

fprintf(fid2,'%s','[');

for i = 1:60,

	if i<10
		xx=strcat('0',num2str(i));
	end

	if i>=10 & i<100
		xx=strcat('',num2str(i));
	end

	if i>99
		xx=num2str(i);
	end

	FileName = strcat('nep_image_',xx,'.png');

fprintf(fid2,'\n%s','{');
fprintf(fid2,'\n%s','"legends":{');
fprintf(fid2,'\n%s','"v1": {"units": "Wave Height (meters)",');
fprintf(fid2,'\n%s','"colormap_tick_labels":{');
fprintf(fid2,'\n%s','"0.0":"0",');
fprintf(fid2,'\n%s','"0.125":"",');
fprintf(fid2,'\n%s','"0.25":"2",');
fprintf(fid2,'\n%s','"0.375":"",');
fprintf(fid2,'\n%s','"0.5":"4",');
fprintf(fid2,'\n%s','"0.625":"",');
fprintf(fid2,'\n%s','"0.75":"6",');
fprintf(fid2,'\n%s','"0.875":"",');
fprintf(fid2,'\n%s','"1.0":"8"}');
fprintf(fid2,'\n%s','},');
fprintf(fid2,'\n%s','"v2": {"units": "Wave Height (feet)",');
fprintf(fid2,'\n%s','"colormap_tick_labels":{');
fprintf(fid2,'\n%s','"0.0":"0",');
fprintf(fid2,'\n%s','"0.125":"",');
fprintf(fid2,'\n%s','"0.25":"7",');
fprintf(fid2,'\n%s','"0.375":"",');
fprintf(fid2,'\n%s','"0.5":"13",');
fprintf(fid2,'\n%s','"0.625":"",');
fprintf(fid2,'\n%s','"0.75":"20",');
fprintf(fid2,'\n%s','"0.875":"",');
fprintf(fid2,'\n%s','"1.0":"26"}');
fprintf(fid2,'\n%s','},');
fprintf(fid2,'\n%s','"default": "v1"');
fprintf(fid2,'\n%s','},');
fprintf(fid2,'\n%s','"colormap":"chelle",');
fprintf(fid2,'\n%s','"colormap_long_pixels":256,');
fprintf(fid2,'\n%s','"model_date":"');
fprintf(fid2,'%s',model_rundate);
fprintf(fid2,'%s','T');
fprintf(fid2,'%s',model_runtime);
fprintf(fid2,'%s','Z",');
fprintf(fid2,'\n%s','"forecast_date":"');
fprintf(fid2,'%s',model_date(i,:));
fprintf(fid2,'%s','T');
fprintf(fid2,'%s',model_time(i,:));
fprintf(fid2,'%s','Z",');
fprintf(fid2,'\n%s','"colormap_short_pixels":16,');
fprintf(fid2,'\n%s','"bbox":[-170.2500,-77.2500,4.7500,60.5000],');
fprintf(fid2,'\n%s','"creation_timestamp":1277208000.0000,');
fprintf(fid2,'\n%s','"image_url":"http://agate.coas.oregonstate.edu/nvs/www/nep/');
fprintf(fid2,'%s',FileName);
fprintf(fid2,'%s','",');
fprintf(fid2,'\n%s','"colorbar_url":"http://agate.coas.oregonstate.edu/nvs/www/image_colorbar.png",');
fprintf(fid2,'\n%s','"var":"wvht"');
fprintf(fid2,'\n%s','},');

end

clear i

for i = 61,

	if i<10
		xx=strcat('0',num2str(i));
	end

	if i>=10 & i<100
		xx=strcat('',num2str(i));
	end

	if i>99
		xx=num2str(i);
	end

	FileName = strcat('nep_image_',xx,'.png');

fprintf(fid2,'\n%s','{');
fprintf(fid2,'\n%s','"legends":{');
fprintf(fid2,'\n%s','"v1": {"units": "Wave Height (meters)",');
fprintf(fid2,'\n%s','"colormap_tick_labels":{');
fprintf(fid2,'\n%s','"0.0":"0",');
fprintf(fid2,'\n%s','"0.125":"",');
fprintf(fid2,'\n%s','"0.25":"2",');
fprintf(fid2,'\n%s','"0.375":"",');
fprintf(fid2,'\n%s','"0.5":"4",');
fprintf(fid2,'\n%s','"0.625":"",');
fprintf(fid2,'\n%s','"0.75":"6",');
fprintf(fid2,'\n%s','"0.875":"",');
fprintf(fid2,'\n%s','"1.0":"8"}');
fprintf(fid2,'\n%s','},');
fprintf(fid2,'\n%s','"v2": {"units": "Wave Height (feet)",');
fprintf(fid2,'\n%s','"colormap_tick_labels":{');
fprintf(fid2,'\n%s','"0.0":"0",');
fprintf(fid2,'\n%s','"0.125":"",');
fprintf(fid2,'\n%s','"0.25":"7",');
fprintf(fid2,'\n%s','"0.375":"",');
fprintf(fid2,'\n%s','"0.5":"13",');
fprintf(fid2,'\n%s','"0.625":"",');
fprintf(fid2,'\n%s','"0.75":"20",');
fprintf(fid2,'\n%s','"0.875":"",');
fprintf(fid2,'\n%s','"1.0":"26"}');
fprintf(fid2,'\n%s','},');
fprintf(fid2,'\n%s','"default": "v1"');
fprintf(fid2,'\n%s','},');
fprintf(fid2,'\n%s','"colormap":"chelle",');
fprintf(fid2,'\n%s','"colormap_long_pixels":256,');
fprintf(fid2,'\n%s','"model_date":"');
fprintf(fid2,'%s',model_rundate);
fprintf(fid2,'%s','T');
fprintf(fid2,'%s',model_runtime);
fprintf(fid2,'%s','Z",');
fprintf(fid2,'\n%s','"forecast_date":"');
fprintf(fid2,'%s',model_date(i,:));
fprintf(fid2,'%s','T');
fprintf(fid2,'%s',model_time(i,:));
fprintf(fid2,'%s','Z",');
fprintf(fid2,'\n%s','"colormap_short_pixels":16,');
fprintf(fid2,'\n%s','"bbox":[-170.2500,-77.2500,4.7500,60.5000],');
fprintf(fid2,'\n%s','"creation_timestamp":1277208000.0000,');
fprintf(fid2,'\n%s','"image_url":"http://agate.coas.oregonstate.edu/nvs/www/nep/');
fprintf(fid2,'%s',FileName);
fprintf(fid2,'%s','",');
fprintf(fid2,'\n%s','"colorbar_url":"http://agate.coas.oregonstate.edu/nvs/www/image_colorbar.png",');
fprintf(fid2,'\n%s','"var":"wvht"');
fprintf(fid2,'\n%s','}');

end

fprintf(fid2,'\n%s',']');

fclose(fid2);

