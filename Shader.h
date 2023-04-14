#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>// ����glew����ȡ���еı���OpenGLͷ�ļ�

using namespace std;

class Shader
{
public:
	// ����ID
	GLuint Program;

	// ��������ȡ��������ɫ��
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// 1.���ļ�·���к�ȥ����/Ƭ����ɫ��
		string  vertexCode;
		string fragmentCode;
		ifstream vShaderFile;
		ifstream fShaderFile;
		// ��֤ifstream��������׳��쳣
		vShaderFile.exceptions(ifstream::badbit);
		fShaderFile.exceptions(ifstream::badbit);
		try {
			// ���ļ�
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			stringstream vShaderStream, fShaderStream;
			// ��ȡ�ļ��Ļ������ݵ�����
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// �ر��ļ�
			vShaderFile.close();
			fShaderFile.close();
			// ת������GLchar����
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (ifstream::failure e) {
			cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
		}
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();

		// 2.������ɫ��
		GLuint vertex, fragment;
		GLint success;
		GLchar infoLog[512];

		// ������ɫ��
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// ��ӡ�����������еĻ���
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
		};

		// Ƭ����ɫ��
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		// ��ӡ�����������еĻ���
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
		};

		// ��ɫ������
		this->Program = glCreateProgram();
		glAttachShader(this->Program, vertex);
		glAttachShader(this->Program, fragment);
		glLinkProgram(this->Program);
		// ��ӡ���Ӵ���
		glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
			cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
		}

		// ɾ����ɫ���������Ѿ����ӵ����ǵĳ����У��Ѿ�������Ҫ��
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// ʹ�ó���
	void Use()
	{
		glUseProgram(this->Program);
	}
};

#endif