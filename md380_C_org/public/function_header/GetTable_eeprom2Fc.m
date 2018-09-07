% 将MATLAB与CCS连接，把 eeprom2FcAutoGenerate[] 读取出来，
% 写入文件 f_table_eeprom2Fc_autoGenerate.c 中。

% 以下1行仅需执行1次
% [boardnum,procnum] = boardprocsel; cc=ticcs('boardnum',boardnum,'procnum',procnum);

% 获取 eeprom2Fc[] 的长度。
sizeOfTable_eeprom2Fc = read(cc,address(cc,'sizeOfTable_eeprom2Fc'),'int16')
sizeOfTable = sizeOfTable_eeprom2Fc;
i = 0:sizeOfTable-1;

% 获取 eeprom2Fc[]
eeprom2FcAutoGenerate = read(cc,address(cc,'eeprom2FcAutoGenerate'),'int16', sizeOfTable);

maxEepromAddr = sizeOfTable_eeprom2Fc-1;


% 写入到文件中
fid = fopen('f_table_eeprom2Fc_autoGenerate.c', 'w+');
fprintf(fid, '// DSP程序自动生成数组，matlab读取CCS中的RAM数据，自动写入文件。\n');

fprintf(fid, '// \n');
fprintf(fid, '// 对应关系表 \n');
fprintf(fid, '// y = eeprom2Fc[i] \n');
fprintf(fid, '// i, 数组下标，----该功能码在EEPROM的位置 \n');
fprintf(fid, '// y, 数组的值，----功能码的序号 \n');
fprintf(fid, '// 功能码总数：    %d \n', sizeOfTable_eeprom2Fc);
fprintf(fid, '// 最大EEPROM地址：%d \n', maxEepromAddr);

fprintf(fid, '// \n');
fprintf(fid, 'const Uint16 eeprom2Fc[] =\n');
fprintf(fid, '{\n');
fprintf(fid, '//  y       // i\n');
fprintf(fid, '// \n');
fprintf(fid, '    %-4d,   // %d\n', [eeprom2FcAutoGenerate; i]);
fprintf(fid, '};\n\n');
fclose(fid);



