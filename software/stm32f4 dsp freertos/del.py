'''
Author: jar-chou 2722642511@qq.com
Date: 2023-09-05 23:59:45
LastEditors: jar-chou 2722642511@qq.com
LastEditTime: 2023-09-06 12:07:19
FilePath: \9.5 22h\del.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
import shutil,os
shutil.rmtree(r'Listing')
os.mkdir(r'Listing')
shutil.rmtree(r'Output')
os.mkdir(r'Output')