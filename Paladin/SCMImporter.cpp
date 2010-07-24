#include "SCMImporter.h"
#include <string.h>

SCMProjectImporterManager::SCMProjectImporterManager(void)
  :	fImporterList(20,true)
{
	fImporterList.AddItem(new SourceforgeImporter());
	fImporterList.AddItem(new BerliosImporter());
	fImporterList.AddItem(new BitbucketImporter());
	fImporterList.AddItem(new GitoriousImporter());
}


SCMProjectImporterManager::~SCMProjectImporterManager(void)
{
}


int32
SCMProjectImporterManager::CountImporters(void) const
{
	return fImporterList.CountItems();
}


SCMProjectImporter *
SCMProjectImporterManager::ImporterAt(const int32 &index)
{
	return fImporterList.ItemAt(index);
}


SCMProjectImporter *
SCMProjectImporterManager::FindImporter(const char *name)
{
	if (!name || strlen(name) == 0)
		return NULL;
	
	for (int32 i = 0; i < fImporterList.CountItems(); i++)
	{
		SCMProjectImporter *item = fImporterList.ItemAt(i);
		if (item && strcmp(item->GetName(),name) == 0)
			return item;
	}
	return NULL;
}


SCMProjectImporter::SCMProjectImporter(void)
  :	fName("SCM Project Importer"),
	fSCM(SCM_NONE)
{
	
}


SCMProjectImporter::~SCMProjectImporter(void)
{

}


const char *
SCMProjectImporter::GetName(void)
{
	return fName.String();
}


void
SCMProjectImporter::SetProjectName(const char *projname)
{
	fProjectName = projname;
}


const char *
SCMProjectImporter::GetProjectName(void)
{
	return fProjectName.String();
}


void
SCMProjectImporter::SetUserName(const char *username)
{
	fUserName = username;
}


const char *
SCMProjectImporter::GetUserName(void)
{
	return fUserName.String();
}


void
SCMProjectImporter::SetURL(const char *url)
{
	fURL = url;
}


const char *
SCMProjectImporter::GetURL(void)
{
	return fURL.String();
}


void
SCMProjectImporter::SetRepository(const char *repo)
{
	fRepository = repo;
}


const char *
SCMProjectImporter::GetRepository(void)
{
	return fRepository.String();
}


void
SCMProjectImporter::SetSCM(const scm_t &scm)
{
	fSCM = scm;
}


scm_t
SCMProjectImporter::GetSCM(void) const
{
	return fSCM;
}


bool
SCMProjectImporter::SupportsSCM(const scm_t &scm) const
{
	return false;
}


void
SCMProjectImporter::SetPath(const char *path)
{
	fPath = path;
}


const char *
SCMProjectImporter::GetPath(void)
{
	return fPath.String();
}


BString
SCMProjectImporter::GetImportCommand(bool readOnly)
{
	// To be implemented by child classes
	return BString();
}


void
SCMProjectImporter::SetName(const char *name)
{
	fName = name;
}


BString
SCMProjectImporter::GetSCMCommand(void)
{
	BString out;
	switch (fSCM)
	{
		case SCM_HG:
		{
			out = "hg";
			break;
		}
		case SCM_GIT:
		{
			out = "git";
			break;
		}
		case SCM_SVN:
		{
			out = "svn";
			break;
		}
		default:
		{
			break;
		}
	}
	return out;
}

#pragma mark -

SourceforgeImporter::SourceforgeImporter(void)
{
	SetName("Sourceforge");
}


BString
SourceforgeImporter::GetImportCommand(bool readOnly)
{
	BString command;
	
	switch (GetSCM())
	{
		case SCM_HG:
		{
			// Read-only:	http://PROJNAME.hg.sourceforge.net:8000/hgroot/PROJNAME/PROJNAME
			// Developer:  ssh://USERNAME@PROJNAME.hg.sourceforge.net/hgroot/PROJNAME/PROJNAME
			if (!readOnly)
				command << "hg clone ssh://" << GetUserName() << "@"
						<< GetProjectName()
						<< ".hg.sourceforge.net/hgroot/" << GetProjectName()
						<< "/" << GetProjectName();
			else
				command << "hg clone http://" << GetProjectName()
						<< ".hg.sourceforge.net:8000/hgroot/" << GetProjectName()
						<< "/" << GetProjectName();
			
			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		case SCM_GIT:
		{
			// Read-only: git://PROJNAME.git.sourceforge.net/gitroot/PROJNAME/REPONAME
			// Developer: ssh://USERNAME@PROJNAME.git.sourceforge.net/gitroot/PROJNAME/REPONAME
			if (!readOnly)
				command << "git clone ssh://" << GetUserName() << "@"
						<< GetProjectName() << ".git.sourceforge.net/gitroot/"
						<< GetProjectName();
			else
				command << "git clone git://" << GetProjectName()
						<< ".git.sourceforge.net/gitroot/"
						<< GetProjectName();
				
			if (GetRepository() && strlen(GetRepository()))
				command << "/" << GetRepository();

			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		case SCM_SVN:
		{
			// Read-only / developer:
			// svn co https://PROJNAME.svn.sourceforge.net/svnroot/PROJNAME FOLDERNAME
			command << "svn co https://" << GetProjectName()
					<< ".svn.sourceforge.net/svnroot/" << GetProjectName()
					<< "/" << GetProjectName();

			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		default:
		{
			// Do nothing
			break;
		}
	}
	return command;
}


bool
SourceforgeImporter::SupportsSCM(const scm_t &scm) const
{
	switch (scm)
	{
		case SCM_HG:
		case SCM_GIT:
		case SCM_SVN:
			return true;
		
		default:
			return false;
	}
}


BerliosImporter::BerliosImporter(void)
{
	SetName("BerliOS");
}


BString
BerliosImporter::GetImportCommand(bool readOnly)
{
	BString command;
	
	switch (GetSCM())
	{
		case SCM_HG:
		{
			// Read-only: http://hg.berlios.de/repos/PROJNAME FOLDERNAME
			// Developer: https://USERNAME@hg.berlios.de/repos/PROJNAME FOLDERNAME
			if (!readOnly)
				command << "hg clone https://" << GetUserName() << "@"
						<< "hg.berlios.de/repos/" << GetProjectName();
			else
				command << "hg clone http://hg.berlios.de/repos/" << GetProjectName();
			
			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		case SCM_GIT:
		{
			// Read-only: git://git.berlios.de/PROJNAME
			// Developer: ssh://USERNAME@git.berlios.de/gitroot/PROJNAME
			if (!readOnly)
				command << "git clone ssh://" << GetUserName() 
						<< "@git.berlios.de/" << GetProjectName();
			else
				command << "git clone git://git.berlios.de/" << GetProjectName();
				
			if (GetRepository() && strlen(GetRepository()))
				command << "/" << GetRepository();

			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		case SCM_SVN:
		{
			// Read-only: svn://svn.berlios.de/PROJNAME/REPONAME
			// developer: svn+ssh://USERNAME@svn.berlios.de/svnroot/repos/PROJNAME/REPONAME
			if (!readOnly)
				command << "svn co svn+ssh://" << GetUserName()
						<< "@svn.berlios.de/svnroot/repos/" << GetProjectName()
						<< "/" << GetRepository();
			else
				command << "svn co svn://svn.berlios.de/" << GetProjectName()
						<< "/" << GetRepository();
			
			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		default:
		{
			// Do nothing
			break;
		}
	}
	return command;
}


bool
BerliosImporter::SupportsSCM(const scm_t &scm) const
{
	switch (scm)
	{
		case SCM_HG:
		case SCM_GIT:
		case SCM_SVN:
			return true;
		
		default:
			return false;
	}
}


BitbucketImporter::BitbucketImporter(void)
{
	SetName("Bitbucket");
}


BString
BitbucketImporter::GetImportCommand(bool readOnly)
{
	BString command;
	
	switch (GetSCM())
	{
		case SCM_HG:
		{
			// read-only: http://bitbucket.org/USERNAME(projectname)/REPONAME
			// developer: ssh://hg@bitbucket.org/USERNAME(projectname)/REPONAME
			if (!readOnly)
				command << "hg clone ssh://hg@bitbucket.org/" << GetProjectName()
						<< "/" << GetRepository();
			else
				command << "hg clone http://bitbucket.org/" << GetProjectName()
						<< "/" << GetRepository();
			
			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		default:
		{
			// Do nothing
			break;
		}
	}
	return command;
}


bool
BitbucketImporter::SupportsSCM(const scm_t &scm) const
{
	return (scm == SCM_HG);
}


GitoriousImporter::GitoriousImporter(void)
{
	SetName("Gitorious");
}


BString
GitoriousImporter::GetImportCommand(bool readOnly)
{
	BString command;
	
	switch (GetSCM())
	{
		case SCM_GIT:
		{
			// read-only: http://git.gitorious.org/PROJNAME/REPONAME.git
			// developer: git://git.gitorious.org/PROJNAME/REPONAME.git
			if (!readOnly)
				command << "git clone git://git.gitorious.org/" << GetProjectName()
						<< "/" << GetRepository() << ".git";
			else
				command << "git clone http://git.gitorious.org/" << GetProjectName()
						<< "/" << GetRepository() << ".git";
			
			if (GetPath() && strlen(GetPath()))
				command << " '" << GetPath() << "'";
			break;
		}
		default:
		{
			// Do nothing
			break;
		}
	}
	return command;
}


bool
GitoriousImporter::SupportsSCM(const scm_t &scm) const
{
	return (scm == SCM_GIT);
}
