#pragma once
#include "..\Main\main.h"
#include "..\Mta_SDK\Common.h"
#include <list>
#include <map>
// Min and max number of characters in player serial
#define MIN_SERIAL_LENGTH 1
#define MAX_SERIAL_LENGTH 32

// Network disconnection reason (message) size
#define NET_DISCONNECT_REASON_SIZE  256

// Element IDs
#define RESERVED_ELEMENT_ID 0xFFFFFFFE
#define INVALID_ELEMENT_ID 0xFFFFFFFF

// Element name characteristics
#define MAX_TYPENAME_LENGTH 32
#define MAX_ELEMENT_NAME_LENGTH 64

// Allow 2^17 server elements and 2^17 client elements
#define MAX_SERVER_ELEMENTS 131072
#define MAX_CLIENT_ELEMENTS 131072


// Make sure MAX_SERVER_ELEMENTS are a power of two
static_assert((MAX_SERVER_ELEMENTS& (MAX_SERVER_ELEMENTS - 1)) == 0, "x");
enum eClientEntityType
{
	CCLIENTCAMERA,
	CCLIENTPLAYER,
	CCLIENTPLAYERMODEL____,
	CCLIENTVEHICLE,
	CCLIENTRADARMARKER,
	CCLIENTOBJECT,
	CCLIENTPICKUP,
	CCLIENTRADARAREA,
	CCLIENTMARKER,
	CCLIENTPATHNODE,
	CCLIENTWORLDMESH,
	CCLIENTTEAM,
	CCLIENTPED,
	CCLIENTPROJECTILE,
	CCLIENTGUI,
	CCLIENTSPAWNPOINT_DEPRECATED,
	CCLIENTCOLSHAPE,
	CCLIENTDUMMY,            // anything user-defined
	SCRIPTFILE,
	CCLIENTDFF,
	CCLIENTCOL,
	CCLIENTTXD,
	CCLIENTSOUND,
	CCLIENTWATER,
	CCLIENTDXFONT,
	CCLIENTGUIFONT,
	CCLIENTTEXTURE,
	CCLIENTSHADER,
	CCLIENTWEAPON,
	CCLIENTEFFECT,
	CCLIENTPOINTLIGHTS,
	CCLIENTSCREENSOURCE,
	CCLIENTRENDERTARGET,
	CCLIENTBROWSER,
	CCLIENTSEARCHLIGHT,
	CCLIENTIFP,
	CCLIENTVECTORGRAPHIC,
	CCLIENTUNKNOWN,
	CCLIENTIMG,
};
class CClientEntity
{
public:
	CClientEntity(ElementID ID);
	virtual ~CClientEntity();

	virtual eClientEntityType GetType() const = 0;
	bool                      IsLocalEntity() { return m_ID >= MAX_SERVER_ELEMENTS; };
	bool                      IsSmartPointer() { return m_bSmartPointer; }
	void                      SetSmartPointer(bool bSmartPointer) { m_bSmartPointer = bSmartPointer; }

	// System entity? A system entity means it can't be removed by the server
	// or the client scripts.
	bool IsSystemEntity() { return m_bSystemEntity; };
	void MakeSystemEntity() { m_bSystemEntity = true; };

	virtual void Unlink() = 0;

	// This is used for realtime synced elements. Whenever a position/rotation change is
	// forced from the server either in form of spawn or setElementPosition/rotation a new
	// value is assigned to this from the server. This value also comes with the sync packets.
	// If this value doesn't match the value from the sync packet, the packet should be
	// ignored. Note that if this value is 0, all sync packets should be accepted. This is
	// so we don't need this byte when the element is created first.
	unsigned char GetSyncTimeContext() { return m_ucSyncTimeContext; };
	void          SetSyncTimeContext(unsigned char ucContext) { m_ucSyncTimeContext = ucContext; }
	bool          CanUpdateSync(unsigned char ucRemote);

	const std::string& GetName() const { return m_strName; }
	void           SetName(const char* szName);

	const void* GetTypeName() { return m_strTypeName; }
	unsigned int   GetTypeHash() { return m_uiTypeHash; }
	static auto    GetTypeHashFromString(void* type);
	void           SetTypeName(const void* name);

	CClientEntity* GetParent() { return m_pParent; };
	CClientEntity* SetParent(CClientEntity* pParent);
	CClientEntity* AddChild(CClientEntity* pChild);
	bool           IsMyChild(CClientEntity* pEntity, bool bRecursive);
	bool           IsMyParent(CClientEntity* pEntity, bool bRecursive);
	bool           IsBeingDeleted() { return m_bBeingDeleted; }
	void           SetBeingDeleted(bool bBeingDeleted) { m_bBeingDeleted = bBeingDeleted; }
	void           ClearChildren();

	void* IterBegin();
	void* IterEnd();
	void* GetChildrenListSnapshot();

	ElementID GetID() { return m_ID; };
	void      SetID(ElementID ID);

	void* GetCustomDataPointer() { return m_pCustomData; }
	void* GetCustomData(const char* szName, bool bInheritData, bool* pbIsSynced = nullptr);
	void* GetAllCustomData(void* table);
	bool           GetCustomDataString(const char* szKey, void* strOut, bool bInheritData);
	bool           GetCustomDataFloat(const char* szKey, float& fOut, bool bInheritData);
	bool           GetCustomDataInt(const char* szKey, int& iOut, bool bInheritData);
	bool           GetCustomDataBool(const char* szKey, bool& bOut, bool bInheritData);
	void           SetCustomData(const char* szName, const void* Variable, bool bSynchronized = true);
	void           DeleteCustomData(const char* szName);

	virtual bool GetMatrix(CVector& matrix) const;
	virtual bool SetMatrix(const CVector& matrix);

	virtual void GetPosition(CVector& vecPosition) const = 0;
	void         GetPositionRelative(CClientEntity* pOrigin, CVector& vecPosition) const;
	virtual void SetPosition(const CVector& vecPosition) = 0;
	virtual void SetPosition(const CVector& vecPosition, bool bResetInterpolation, bool bAllowGroundLoadFreeze = true) { SetPosition(vecPosition); }
	void         SetPositionRelative(CClientEntity* pOrigin, const CVector& vecPosition);
	virtual void Teleport(const CVector& vecPosition) { SetPosition(vecPosition); }

	virtual void GetRotationRadians(CVector& vecOutRadians) const;
	virtual void GetRotationDegrees(CVector& vecOutDegrees) const;
	virtual void SetRotationRadians(const CVector& vecRadians);
	virtual void SetRotationDegrees(const CVector& vecDegrees);

	virtual inline unsigned short GetDimension() { return m_usDimension; }
	virtual void                  SetDimension(unsigned short usDimension);

	virtual void ModelRequestCallback(void* pModelInfo) {};

	virtual bool IsOutOfBounds();
	void* GetModelInfo() { return m_pModelInfo; };

	CClientEntity* GetAttachedTo() { return m_pAttachedToEntity; }
	virtual void   AttachTo(CClientEntity* pEntity);
	virtual void   GetAttachedOffsets(CVector& vecPosition, CVector& vecRotation);
	virtual void   SetAttachedOffsets(CVector& vecPosition, CVector& vecRotation);
	bool           IsEntityAttached(CClientEntity* pEntity);
	bool           IsAttachedToElement(CClientEntity* pEntity, bool bRecursive = true);
	unsigned int            GetAttachedEntityCount() { return m_AttachedEntities.size(); }
	CClientEntity* GetAttachedEntity(unsigned int  uiIndex) { return m_AttachedEntities[uiIndex]; }
	void           ReattachEntities();
	virtual bool   IsAttachable();
	virtual bool   IsAttachToable();
	virtual void   DoAttaching();

	bool AddEvent(void* pLuaMain, const char* szName, const void* iLuaFunction, bool bPropagated, void* eventPriority,
		float fPriorityMod);
	bool CallEvent(const char* szName, const void* Arguments, bool bCallOnChildren);
	void CallEventNoParent(const char* szName, const void* Arguments, CClientEntity* pSource);
	void CallParentEvent(const char* szName, const void* Arguments, CClientEntity* pSource);
	bool DeleteEvent(void* pLuaMain, const char* szName, const void* iLuaFunction);
	void DeleteEvents(void* pLuaMain, bool bRecursive);
	void DeleteAllEvents();

	void CleanUpForVM(void* pLuaMain, bool bRecursive);

	CClientEntity* FindChild(const char* szName, unsigned int uiIndex, bool bRecursive);
	CClientEntity* FindChildIndex(const char* szType, unsigned int uiIndex, unsigned int& uiCurrentIndex, bool bRecursive);
	CClientEntity* FindChildByType(const char* szType, unsigned int uiIndex, bool bRecursive);
	CClientEntity* FindChildByTypeIndex(unsigned int uiTypeHash, unsigned int uiIndex, unsigned int& uiCurrentIndex, bool bRecursive);
	void           FindAllChildrenByType(const char* szType, struct lua_State* luaVM, bool bStreamedIn = false);
	void           FindAllChildrenByTypeIndex(unsigned int uiTypeHash, lua_State* luaVM, unsigned int& uiIndex, bool bStreamedIn = false);

	unsigned int CountChildren();

	void GetChildren(lua_State* luaVM);
	void GetChildrenByType(const char* szType, lua_State* luaVM);

	void AddCollision(void* pShape);
	void RemoveCollision(void* pShape);
	bool                                  CollisionExists(void* pShape);
	void                                  RemoveAllCollisions();
	void* CollisionsBegin();
	void* CollisionsEnd();

	void* GetElementGroup() { return m_pElementGroup; }
	void           SetElementGroup(void* elementGroup) { m_pElementGroup = elementGroup; }

	static unsigned int GetTypeID(const char* szTypeName);

	void* GetEventManager() { return m_pEventManager; };

	void DeleteClientChildren();

	// Returns true if this class is inherited by CClientStreamElement
	virtual bool IsStreamingCompatibleClass() { return false; };

	void AddOriginSourceUser(void* pModel) { m_OriginSourceUsers.push_back(pModel); }
	void RemoveOriginSourceUser(void* pModel) { m_OriginSourceUsers.remove(pModel); }

	void AddContact(void* pModel) { m_Contacts.push_back(pModel); }
	void RemoveContact(void* pModel);

	virtual void* GetGameEntity() { return NULL; }
	virtual const void* GetGameEntity() const { return NULL; }

	bool IsCollidableWith(CClientEntity* pEntity);
	void SetCollidableWith(CClientEntity* pEntity, bool bCanCollide);

	bool IsDoubleSided();
	void SetDoubleSided(bool bEnable);

	// Game layer functions for CEntity/CPhysical
	virtual void     InternalAttachTo(CClientEntity* pEntity);
	bool             IsStatic();
	void             SetStatic(bool bStatic);
	unsigned char    GetInterior();
	virtual void     SetInterior(unsigned char ucInterior);
	bool             IsOnScreen();
	virtual void* GetClump();
	void             WorldIgnore(bool bIgnore);

	// Spatial database
	virtual void* GetWorldBoundingSphere();
	virtual void    UpdateSpatialData();

	virtual void DebugRender(const CVector& vecPosition, float fDrawRadius) {}

	float GetDistanceBetweenBoundingSpheres(CClientEntity* pOther);

	bool         IsCallPropagationEnabled() { return m_bCallPropagationEnabled; }
	virtual void SetCallPropagationEnabled(bool bEnabled) { m_bCallPropagationEnabled = bEnabled; }

	bool CanBeDestroyedByScript() { return m_canBeDestroyedByScript; }
	void SetCanBeDestroyedByScript(bool canBeDestroyedByScript) { m_canBeDestroyedByScript = canBeDestroyedByScript; }

protected:
	void* m_pManager;
	CClientEntity* m_pParent;
	void* m_Children;
	void* m_pChildrenListSnapshot;
	unsigned int                    m_uiChildrenListSnapshotRevision;

	void* m_pCustomData;

	ElementID m_ID;
	CVector   m_vecRelativePosition;

	unsigned short m_usDimension;

	unsigned int m_uiLine;

private:
	unsigned int m_uiTypeHash;
	void* m_strTypeName;
	std::string m_strName;
	bool         m_bSmartPointer;

protected:
	unsigned char m_ucSyncTimeContext;

	CClientEntity* m_pAttachedToEntity;
	CVector                     m_vecAttachedPosition;
	CVector                     m_vecAttachedRotation;
	std::vector<CClientEntity*> m_AttachedEntities;
	bool                        m_bDisallowAttaching;            // Protect against attaching in destructor

	bool                              m_bBeingDeleted;
	bool                              m_bSystemEntity;
	void* m_pEventManager;
	void* m_pModelInfo;
	void* m_Collisions;
	void* m_pElementGroup;
	std::list<void*>            m_OriginSourceUsers;
	std::vector<void*>          m_Contacts;
	unsigned char                     m_ucInterior;
	std::map<CClientEntity*, bool>    m_DisabledCollisions;
	bool                              m_bDoubleSided;
	bool                              m_bDoubleSidedInit;
	bool                              m_bWorldIgnored;
	bool                              m_bCallPropagationEnabled;
	bool                              m_bDisallowCollisions;
	bool                              m_canBeDestroyedByScript = true;            // If true, destroyElement function will
	// have no effect on this element
public:
	// Optimization for getElementsByType starting at root
	static void StartupEntitiesFromRoot();

private:
	static bool IsFromRoot(CClientEntity* pEntity);
	static void AddEntityFromRoot(unsigned int uiTypeHash, CClientEntity* pEntity, bool bDebugCheck = true);
	static void RemoveEntityFromRoot(unsigned int uiTypeHash, CClientEntity* pEntity);
	static void GetEntitiesFromRoot(unsigned int uiTypeHash, lua_State* luaVM, bool bStreamedIn);

#if CHECK_ENTITIES_FROM_ROOT
	static void _CheckEntitiesFromRoot(unsigned int uiTypeHash);
	void        _FindAllChildrenByTypeIndex(unsigned int uiTypeHash, std::map<CClientEntity*, int>& mapResults);
	static void _GetEntitiesFromRoot(unsigned int uiTypeHash, std::map<CClientEntity*, int>& mapResults);
#endif
};