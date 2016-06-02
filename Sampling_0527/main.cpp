
#include "FileZ.h"
#include "model.h"
#include "readbmp.h"
#include "windows.h"
#include "direct.h"
#include "time.h"

using namespace std;

GLfloat EACHANGLE = 0;
GLint PATCHSIZE = 0;
GLMmodel **model;
double myd = 0.0;
GLfloat angle = 0;
GLfloat angledoor = 0;

int model_num = 0;
int model_index = 0;
int seed_index = 0;
int snum = 0;
int view_num = 0;
string black_bmp_path;
string black_patch_path;
string models_path;
string seed_path;

string render_path;
FileZ fz("");

void initial()
{
	
	model = new GLMmodel*[model_num];

}
void del()
{
	for (int i = 0; i<model_num; i++)
	{
		glmDelete(model[i]);
	}
	delete[] model;
}

void GL_myInitial()
{
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	//GLfloat light_ambient [] = {0.75f, 0.75f, 0.75f, 1.0f};
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
}

//视角设置
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(60.0,(GLfloat)w/(GLfloat)h,1.0,20.0);
	glOrtho(-2, 2, -2, 2, 1.0, 20.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, -1.732 * 2, 1 * 2,
		0, 0, 0,
		0, 1 * 2, 1.732 * 2);
}

BOOL WriteBitmapFile(const char * filename, int width, int height, unsigned char * bitmapData)
{
	//填充BITMAPFILEHEADER
	BITMAPFILEHEADER bitmapFileHeader;
	memset(&bitmapFileHeader, 0, sizeof(BITMAPFILEHEADER));
	bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER);
	bitmapFileHeader.bfType = 0x4d42;	//BM
	bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//填充BITMAPINFOHEADER
	BITMAPINFOHEADER bitmapInfoHeader;
	memset(&bitmapInfoHeader, 0, sizeof(BITMAPINFOHEADER));
	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biWidth = width;
	bitmapInfoHeader.biHeight = height;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB;
	bitmapInfoHeader.biSizeImage = width * abs(height) * 3;

	//////////////////////////////////////////////////////////////////////////
	FILE * filePtr;			//连接要保存的bitmap文件用
	unsigned char tempRGB;	//临时色素
	int imageIdx;

	//交换R、B的像素位置,bitmap的文件放置的是BGR,内存的是RGB
	for (imageIdx = 0; imageIdx < bitmapInfoHeader.biSizeImage; imageIdx += 3)
	{
		tempRGB = bitmapData[imageIdx];
		bitmapData[imageIdx] = bitmapData[imageIdx + 2];
		bitmapData[imageIdx + 2] = tempRGB;
	}

	filePtr = fopen(filename, "wb");
	if (NULL == filePtr)
	{
		return FALSE;
	}

	fwrite(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

	fwrite(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

	fwrite(bitmapData, bitmapInfoHeader.biSizeImage, 1, filePtr);

	fclose(filePtr);
	return TRUE;
}
void SaveScreenShot(int clnHeight, int clnWidth)
{
	//int clnHeight,clnWidth;	//client width and height
	static void * screenData;
	//  RECT rc;
	int len = clnWidth * clnHeight * 3;
	screenData = malloc(len);
	memset(screenData, 0, len);
	glReadPixels(0, 0, clnWidth, clnHeight, GL_RGB, GL_UNSIGNED_BYTE, screenData);

	/*int blackindex = (model_index)*view_num + angle / EACHANGLE + 1;
	string blackbmpfilename = black_bmp_path + "\\"
	+ std::to_string(blackindex) + ".bmp";*/
	int blackindex = angle / EACHANGLE + 1;
	string blackbmpfilename = black_bmp_path + "\\" + fz.subfile[model_index].substr(0,fz.subfile[model_index].find_first_of('.'))
		+ "_" + std::to_string(0) + std::to_string(blackindex) + ".bmp";

	BmpImage* image = readbmp(blackbmpfilename);

	Patch * mypatch = getpatch((unsigned char *)screenData, clnWidth, clnHeight);
	//如果看得到被选中的区域
	if (mypatch->x1 > -0.5)
	{
		//左下角点
		BmpImage* blackpatch = NULL;
		if (ceil(mypatch->x1*image->width) + PATCHSIZE - 1 <= image->width && ceil(mypatch->y1*image->height) + PATCHSIZE - 1 <= image->height)
			blackpatch = imcrop(image, (int)ceil(mypatch->x1*image->width), (int)ceil(mypatch->y1*image->height), PATCHSIZE - 1);
		//右下角
		else if (mypatch->x2*image->width - PATCHSIZE + 1 >= 1 && ceil(mypatch->y1*image->height) + PATCHSIZE - 1 <= image->height)
			blackpatch = imcrop(image, (int)floor(mypatch->x2*image->width - PATCHSIZE + 1), (int)ceil(mypatch->y1*image->height), PATCHSIZE - 1);
		//左上角
		else if (ceil(mypatch->x1*image->width) + PATCHSIZE - 1 <= image->width && mypatch->y2*image->height - PATCHSIZE + 1 >= 1)
			blackpatch = imcrop(image, (int)ceil(mypatch->x1*image->width), (int)floor(mypatch->y2*image->height - PATCHSIZE + 1), PATCHSIZE - 1);
		//右上角
		else if (mypatch->x2*image->width - PATCHSIZE + 1 >= 1 && mypatch->y2*image->height - PATCHSIZE + 1 >= 1)
			blackpatch = imcrop(image, (int)floor(mypatch->x2*image->width - PATCHSIZE + 1), (int)floor(mypatch->y2*image->height - PATCHSIZE + 1), PATCHSIZE - 1);
		else//图片太小
		{
			free(screenData);
			return;
		}
		//*(model[model_index]->objname)+"_"+
		string filename = black_patch_path + "\\" + std::to_string((int)(angle / EACHANGLE + 1)) + "\\" +
			fz.subfile[model_index].substr(0, fz.subfile[model_index].find_first_of('.')) + "_" +
			std::to_string((seed_index + 1)) + ".bmp";
		WriteBitmapFile(filename.data(), PATCHSIZE, PATCHSIZE, (unsigned char*)blackpatch->dataOfBmp);
		free(blackpatch->dataOfBmp);
		free(blackpatch);

		string render_file = render_path + "\\" + std::to_string((int)(angle / EACHANGLE + 1)) + "\\" +
			fz.subfile[model_index].substr(0, fz.subfile[model_index].find_first_of('.')) + "_" +
			std::to_string((seed_index + 1)) + ".bmp";
		WriteBitmapFile(render_file.data(), clnWidth, clnHeight, (unsigned char*)screenData);
	}
	////生成文件名字符串，以时间命名
	//time_t tm = 0;
	//tm = time(NULL);
	//char lpstrFilename[256] = {0};
	//sprintf_s(lpstrFilename,sizeof(lpstrFilename),"%d.bmp",tm);

	//WriteBitmapFile(filename.data(),clnWidth,clnHeight,(unsigned char*)screenData);
	//释放内存
	free(image->dataOfBmp);
	free(image);
	free(mypatch);
	free(screenData);
}
//显示模型
void renderScene()
{
	/*if(!model[model_index]->isstyle[seed_index])
	return;*/
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glShadeModel(GL_SMOOTH);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDisable(GL_LIGHT1);

	GLfloat mat_ambient[] = { 32.0 / 255.0f, 32.0 / 255.0f, 32.0 / 255.0f, 1.0f };
	GLfloat mat_diffuse[] = { 182.0 / 255.0f, 182.0 / 255.0f, 182.0 / 255.0f, 1.0f };
	GLfloat mat_specular[] = { 0.6f, 0.6f, 0.6f, 1.0f };


	//GLfloat mat_ambient[] = { 32.0/255.0f, 32.0/255.0f, 32.0/255.0f, 1.0f };
	//GLfloat mat_diffuse[] = { 1.0f,0.0f, 0.0f, 1.0f };
	//GLfloat mat_specular[] = { 0.6f, 0.2f, 0.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, mat_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, mat_specular);

	glLightfv(GL_LIGHT1, GL_AMBIENT, mat_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, mat_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, mat_specular);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glDisable(GL_CULL_FACE);

	glPushMatrix();
	glRotatef((GLfloat)angle, 0, 0, 1);
	for (int i = 0; i<model[model_index]->numtriangles; i++)
	{
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 

		glBegin(GL_TRIANGLES);

		if (model[model_index]->iscolored[seed_index*model[model_index]->numtriangles + i] == false)
		{
			glColor3f(0.71f, 0.71f, 0.71f);
		}
		else
		{
			glColor3f(0.0f, 0.0f, 8.0f);
		}


		int ifindex = model[model_index]->triangles[i].findex;
		glNormal3f(model[model_index]->facetnorms[3 * ifindex], model[model_index]->facetnorms[3 * ifindex + 1], model[model_index]->facetnorms[3 * ifindex + 2]);

		glVertex3f(model[model_index]->vertices[model[model_index]->triangles[i].vindices[0] * 3 + 0],
			model[model_index]->vertices[model[model_index]->triangles[i].vindices[0] * 3 + 1],
			model[model_index]->vertices[model[model_index]->triangles[i].vindices[0] * 3 + 2]);

		glVertex3f(model[model_index]->vertices[model[model_index]->triangles[i].vindices[1] * 3 + 0],
			model[model_index]->vertices[model[model_index]->triangles[i].vindices[1] * 3 + 1],
			model[model_index]->vertices[model[model_index]->triangles[i].vindices[1] * 3 + 2]);

		glVertex3f(model[model_index]->vertices[model[model_index]->triangles[i].vindices[2] * 3 + 0],
			model[model_index]->vertices[model[model_index]->triangles[i].vindices[2] * 3 + 1],
			model[model_index]->vertices[model[model_index]->triangles[i].vindices[2] * 3 + 2]);


		glEnd();
	}

	glPopMatrix();
	SaveScreenShot(700, 700);
	glutSwapBuffers();
	//Sleep(1000);
	//glutPostRedisplay();
}

void Update(void) {

	//Sleep(100);
	//if(model[model_index]->isstyle[seed_index])
	glutPostRedisplay();

	//if(angle > 329.0)//当所有视角转了一遍

	if (angle > angledoor)
	{
		angle = 0;
		seed_index++;
		if (seed_index == snum)//当该模型的所有种子遍历了
		{
			cout << model[model_index]->objname->data() << "绘制完成" << endl;
			seed_index = 0;
			model_index++;

			if (model_index == model_num)
				exit(0);
		}
	}
	else{
		angle = angle + EACHANGLE;

	}
}

//将文件内容读到数组中去 
void getFiles(string path)
{

	long   hFile = 0;
	int i = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
				continue;

			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
			{
				string filename = fileinfo.name;
				//.obj file
				if (filename.substr(filename.find_first_of(".")) == ".obj")
				{
					string temp = path + "\\" + filename;
					cout << "reading " << temp << endl;
					model[i] = glmReadOBJ(const_cast<char *>(temp.c_str()), filename);
					glmFacetNormals(model[i]);
					i++;
				}
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

//设置随机种子并着色
void setseed()
{
	cout << "设置种子" << endl;

	for (int k = 0; k<model_num; k++)
	{
		model[k]->numseeds = snum;
		model[k]->seeds = (GLfloat *)malloc(sizeof(GLfloat) *
			3 * (model[k]->numseeds));

		model[k]->iscolored = new bool[model[k]->numseeds * model[k]->numtriangles];
		for (int i = 0; i<(model[k]->numseeds * model[k]->numtriangles); i++)
			model[k]->iscolored[i] = false;

		for (int i = 0; i<model[k]->numseeds; i++)
		{
			//srand((unsigned)time(NULL));
			int index = rand() % model[k]->numtriangles;
			int temp = 0;
			while (temp<1000)
			{
				bool tag = true;
				for (int ttt = 0; ttt<model[k]->numseeds; ttt++){
					//如果已经被涂色，抛弃这个种子
					if (model[k]->iscolored[ttt*model[k]->numtriangles + index])
					{
						tag = false;
						break;
					}
				}
				//如果这个种子可以用
				if (tag)
					break;
				else
				{
					index = rand() % model[k]->numtriangles;
					temp++;
				}
			}
			model[k]->seeds[3 * i + 0] = model[k]->vertices[model[k]->triangles[index].vindices[0] * 3 + 0];
			model[k]->seeds[3 * i + 1] = model[k]->vertices[model[k]->triangles[index].vindices[0] * 3 + 1];
			model[k]->seeds[3 * i + 2] = model[k]->vertices[model[k]->triangles[index].vindices[0] * 3 + 2];

			for (int j = 0; j<model[k]->numtriangles; j++)
			{
				double distance0 = pow(model[k]->vertices[model[k]->triangles[j].vindices[0] * 3 + 0] - model[k]->seeds[3 * i + 0], 2) +
					pow(model[k]->vertices[model[k]->triangles[j].vindices[0] * 3 + 1] - model[k]->seeds[3 * i + 1], 2) +
					pow(model[k]->vertices[model[k]->triangles[j].vindices[0] * 3 + 2] - model[k]->seeds[3 * i + 2], 2);

				double distance1 = pow(model[k]->vertices[model[k]->triangles[j].vindices[1] * 3 + 0] - model[k]->seeds[3 * i + 0], 2) +
					pow(model[k]->vertices[model[k]->triangles[j].vindices[1] * 3 + 1] - model[k]->seeds[3 * i + 1], 2) +
					pow(model[k]->vertices[model[k]->triangles[j].vindices[1] * 3 + 2] - model[k]->seeds[3 * i + 2], 2);

				double distance2 = pow(model[k]->vertices[model[k]->triangles[j].vindices[2] * 3 + 0] - model[k]->seeds[3 * i + 0], 2) +
					pow(model[k]->vertices[model[k]->triangles[j].vindices[2] * 3 + 1] - model[k]->seeds[3 * i + 1], 2) +
					pow(model[k]->vertices[model[k]->triangles[j].vindices[2] * 3 + 2] - model[k]->seeds[3 * i + 2], 2);
				//需要所有点都在范围内
				if (distance0 < myd && distance1 < myd && distance2 <myd)
				{
					model[k]->iscolored[i*model[k]->numtriangles + j] = true;
				}
			}
		}
	}
}

void writeseed(string filename)
{
	ofstream ofile;               
	ofile.open(filename);    
	for (int i = 0; i<model_num; i++)
	{
		ofile << model[i]->objname->data() << endl;
		for (int j = 0; j<model[i]->numseeds; j++)
		{
			ofile << model[i]->seeds[3 * j + 0] << " " << model[i]->seeds[3 * j + 1] << " " << model[i]->seeds[3 * j + 2] << endl;
		}
	}
	ofile.close();                
}


int main(int argc, char * argv[])
{


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(700, 100);     
	glutCreateWindow("Sampling parts");
	//GL_myInitial();//一种光照方法
	
	clock_t start = clock();
	ifstream ifs;
	ifs.open("params.cfg");
	if (ifs.fail())
	{
		cout << "can't open parameters file" << endl;
	}
	string tmp;
	ifs >> tmp >> black_bmp_path 
		>> tmp >> black_patch_path
		>> tmp >> models_path
		>> tmp >> render_path
		>> tmp >> seed_path
		>> tmp >> model_num
		>> tmp >> snum
		>> tmp >> view_num
		>> tmp >> angledoor
		>> tmp >> PATCHSIZE
		>> tmp >> EACHANGLE;
	ifs.close();

	for (int i = 1; i <= view_num; i++)
	{
		string temp = black_patch_path + "\\" + to_string(i);
		FileZ fz(temp);
		if (!fz.isExist())
		{
			_mkdir(temp.data());
		}
	}

	fz.name = models_path;
	fz.getFiles();

	initial();

	//load all the models
	getFiles(models_path);

	angle = -EACHANGLE;
	myd = pow(0.3, 2) * 3;
	setseed();
	writeseed(seed_path);

	//isincube();//给指定区域着色

	glutDisplayFunc(renderScene);
	glutReshapeFunc(reshape);
	glutIdleFunc(&Update);
	glutMainLoop();
	
	del();

	clock_t end = clock();
	cout << "it takes " << (end - start) / 1000 << "seconds" << endl;
	cin.get();
	return 0;

}