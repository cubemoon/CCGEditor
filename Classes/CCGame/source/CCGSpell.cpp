#include "CCGSpell.h"
#include "CCGLuaCache.h"

USING_NS_CC;

CCGSpell::CCGSpell(void)
{

}

CCGSpell::~CCGSpell(void)
{
	removeAllChildren();
	dettachLuaScript();
}

bool CCGSpell::initAdditional(void)
{
	CCGSprite::initAdditional();
	scheduleUpdate();
	setGlobalZOrder(ZORDER_SPELL);

	CCString* shaderCode = CCString::createWithContentsOfFile(std::string("Shaders/tansparent.fsh"));

	// ���ض�����ɫ����ƬԪ��ɫ��
	_pShaderProgram = new CCGLProgram();
	_pShaderProgram->initWithVertexShaderByteArray(ccPositionTextureA8Color_vert, shaderCode->getCString());

	// ���ö�����ɫ����attribute���������ꡢ�������ꡢ��ɫ
	_pShaderProgram->addAttribute(GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION);
	_pShaderProgram->addAttribute(GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR);
	_pShaderProgram->addAttribute(GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORD);

	// �Զ�����ɫ������
	_pShaderProgram->link();

	// �����ƶ������š���ת����
	_pShaderProgram->updateUniforms();

	return true;
}

void CCGSpell::draw(Renderer *renderer, const Mat4& transform, uint32_t flags)
{
	_insideBounds = (flags & FLAGS_TRANSFORM_DIRTY) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;

	if (_insideBounds)
	{
		CCASSERT(!_batchNode, "If Sprite is being rendered by CCSpriteBatchNode, CCSprite::draw SHOULD NOT be called");

		_customCommand.init(1);
		_customCommand.func = CC_CALLBACK_0(CCGSpell::onDraw, this, transform, flags);
		renderer->addCommand(&_customCommand);
	}
}

void CCGSpell::onDraw(const Mat4 &transform, uint32_t flags)
{
	// ����attributes�������룬�������꣬�������꣬��ɫ
	ccGLEnableVertexAttribs(kCCVertexAttribFlag_PosColorTex);

	_pShaderProgram->use();
	_pShaderProgram->setUniformsForBuiltins(transform);

	// �����������0
	ccGLBindTexture2D(_texture->getName());
	long offset = (long)&_quad;

	// vertex
	int diff = offsetof(V3F_C4B_T2F, vertices);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (void*)(offset + diff));

	// texCoods
	diff = offsetof(V3F_C4B_T2F, texCoords);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (void*)(offset + diff));

	// color
	diff = offsetof(V3F_C4B_T2F, colors);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (void*)(offset + diff));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CC_INCREMENT_GL_DRAWS(1);
}

void CCGSpell::runLuaScript(std::string scriptName)
{
	CCGLuaCache* luaCache = CCGLuaCache::getInstance();
#ifdef CCG_LUA_DEBUG
	_luaHandle = CCGLuaEngine::create(luaCache->getScriptPath(scriptName)->getCString(), static_cast<CCGSprite*>(this));
#else
	_luaHandle = CCGLuaEngine::create(luaCache->getScript(scriptName), static_cast<CCGSprite*>(this));
#endif
	_luaHandle->callLuaStart();
}

void CCGSpell::dettachLuaScript()
{
	_luaHandle = nullptr;
}

void CCGSpell::update(float delta)
{
	if (_luaHandle != nullptr)
	{
		_luaHandle->callLuaUpdate(delta);
	}
}