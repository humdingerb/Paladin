/*
 * Copyright 2018 Adam Fowler <adamfowleruk@gmail.com>
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Adam Fowler, adamfowleruk@gmail.com
 */
#include "SCMImportWindow.h"

#include <Catalog.h>
#include <Directory.h>
#include <ScrollView.h>
#include <String.h>

#include "DPath.h"
#include "Globals.h"
#include "GitSourceControl.h"
#include "HgSourceControl.h"
#include "SCMOutputWindow.h"
#include "SourceControl.h"
#include "SVNSourceControl.h"

#include <LayoutBuilder.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SCMImportWindow"

enum
{
	M_USE_PROVIDER = 'uspr',
	M_USE_CUSTOM_PROVIDER = 'uscp',
	M_TOGGLE_ANONYMOUS = 'tgan',
	M_UPDATE_COMMAND = 'ucmd',
	M_SCM_IMPORT = 'scmi'
};

SCMImportWindow::SCMImportWindow(void)
  :	DWindow(BRect(0,0,350,350), B_TRANSLATE("Import from repository"))
{
	MakeCenteredOnShow(true);
	
	BMenu *menu = new BMenu(B_TRANSLATE("Providers"));
	
	for (int32 i = 0; i < fProviderMgr.CountImporters(); i++)
	{
		SCMProjectImporter *importer = fProviderMgr.ImporterAt(i);
		if (!importer)
			continue;
		
		BMessage *msg = new BMessage(M_USE_PROVIDER);
		menu->AddItem(new BMenuItem(importer->GetName(), msg));
	}
	
	// Disable custom commands for now	
//	menu->AddSeparatorItem();
//	menu->AddItem(new BMenuItem("Custom", new BMessage(M_USE_CUSTOM_PROVIDER)));
	
	menu->SetLabelFromMarked(true);
	menu->ItemAt(0L)->SetMarked(true);
	
	fProviderField = new BMenuField("repofield", B_TRANSLATE("Provider:"), menu);
	
	menu = new BMenu(B_TRANSLATE("Methods"));
	if (gHgAvailable)
		menu->AddItem(new BMenuItem(B_TRANSLATE("Mercurial"), new BMessage(M_UPDATE_COMMAND)));
	
	if (gGitAvailable)
		menu->AddItem(new BMenuItem(B_TRANSLATE("Git"), new BMessage(M_UPDATE_COMMAND)));
	
	if (gSvnAvailable)
		menu->AddItem(new BMenuItem(B_TRANSLATE("Subversion"), new BMessage(M_UPDATE_COMMAND)));
	menu->SetLabelFromMarked(true);
	menu->ItemAt(0L)->SetMarked(true);
	fProvider = fProviderMgr.ImporterAt(0);
		
	fSCMField = new BMenuField("scmfield", B_TRANSLATE("Method:"), menu);
		
	fProjectBox = new AutoTextControl("project", B_TRANSLATE("Project:"), "",
									new BMessage(M_UPDATE_COMMAND));
	
	fAnonymousBox = new BCheckBox("anonymous", B_TRANSLATE("Anonymous check-out"),
									new BMessage(M_TOGGLE_ANONYMOUS));
	fAnonymousBox->SetValue(B_CONTROL_ON);
	
	fUserNameBox = new AutoTextControl("username", B_TRANSLATE("Username:"), "",
									new BMessage(M_UPDATE_COMMAND));
	fUserNameBox->SetEnabled(false);
	
	fRepository = new AutoTextControl("repository", B_TRANSLATE("Repository owner:"), "",
									new BMessage(M_UPDATE_COMMAND));	
	
	fCommandLabel = new BStringView("commandlabel", B_TRANSLATE("Command:"));	
	fCommandView = new BTextView("command");
	
	BScrollView *scroll = new BScrollView("scrollview", fCommandView,
											0, false, true);
	fCommandView->MakeEditable(false);
	
	fOK = new BButton("ok", B_TRANSLATE("Import"), new BMessage(M_SCM_IMPORT));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(10)
		.Add(fProviderField)
		.Add(fSCMField)
		.Add(fRepository)
		.Add(fProjectBox)
		.Add(fAnonymousBox)
		.Add(fUserNameBox)
		.Add(fCommandLabel)
		.Add(scroll)
		.Add(fOK)
	.End();
		
	fOK->MakeDefault(true);
	fOK->SetEnabled(false);
	
	UpdateCommand();
	fProviderField->MakeFocus(true);
	
}


void
SCMImportWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case M_USE_PROVIDER:
		{
			SCMProjectImporter *importer = fProviderMgr.ImporterAt(
												fProviderField->Menu()->IndexOf(
													fProviderField->Menu()->FindMarked()));
			if (importer)
				SetProvider(importer);
			break;
		}
		case M_USE_CUSTOM_PROVIDER:
		{
			SetProvider(NULL);
			break;
		}
		case M_TOGGLE_ANONYMOUS:
		{
			fUserNameBox->SetEnabled(fAnonymousBox->Value() != B_CONTROL_ON);
			UpdateCommand();
			break;
		}
		case M_UPDATE_COMMAND:
		{
			UpdateCommand();
			break;
		}
		case M_SCM_IMPORT:
		{
			DoImport();
			break;
		}
		default:
		{
			DWindow::MessageReceived(msg);
			break;
		}
	}
}


void
SCMImportWindow::FrameResized(float w, float h)
{
	BRect textRect = fCommandView->Bounds().InsetByCopy(10.0, 10.0);
	fCommandView->SetTextRect(textRect);
}


void
SCMImportWindow::SetProvider(SCMProjectImporter *importer)
{
	if (importer && !fProvider)
	{
		fSCMField->SetEnabled(true);
		fProjectBox->SetEnabled(true);
		fAnonymousBox->SetEnabled(true);
		fUserNameBox->SetEnabled(true);
		fRepository->SetEnabled(true);
		fCommandView->MakeEditable(false);
		fOK->SetEnabled(false);
		fProjectBox->MakeFocus(true);
		fProvider = importer;
		
		UpdateCommand();
	}
	else if (!importer && fProvider)
	{
		fSCMField->SetEnabled(false);
		fProjectBox->SetEnabled(false);
		fAnonymousBox->SetEnabled(false);
		fUserNameBox->SetEnabled(false);
		fRepository->SetEnabled(false);
		fCommandView->MakeEditable(true);
		fCommandView->SetText("");
		fCommandView->MakeFocus(true);
		fOK->SetEnabled(true);
	}
	
	fProvider = importer;
	if (fProvider)
	{
		BMenu *menu = fSCMField->Menu();
		BMenuItem *item;
		
		item = menu->FindItem(B_TRANSLATE("Mercurial"));
		if (item)
			item->SetEnabled(fProvider->SupportsSCM(SCM_HG));
			
		item = menu->FindItem(B_TRANSLATE("Git"));
		if (item)
			item->SetEnabled(fProvider->SupportsSCM(SCM_GIT));
		
		item = menu->FindItem(B_TRANSLATE("Subversion"));
		if (item)
			item->SetEnabled(fProvider->SupportsSCM(SCM_SVN));
		
		item = menu->FindMarked();
		if (!item->IsEnabled())
		{
			item->SetMarked(false);
			for (int32 i = 0; i < menu->CountItems(); i++)
				if (menu->ItemAt(i)->IsEnabled())
				{
					menu->ItemAt(i)->SetMarked(true);
					break;
			}
		}
	}
	UpdateCommand();
}


void
SCMImportWindow::UpdateCommand(void)
{
	if (!fProjectBox->Text() || strlen(fProjectBox->Text()) < 1)
	{
		if (fOK->IsEnabled())
			fOK->SetEnabled(false);
		
		fCommandView->SetText("");
		return;
	}
	else
		if (!fOK->IsEnabled())
			fOK->SetEnabled(true);
	
	fProvider->SetProjectName(fProjectBox->Text());
	fProvider->SetUserName(fUserNameBox->Text());
	fProvider->SetRepository(fRepository->Text());
	
	BMenuItem *item = fSCMField->Menu()->FindMarked();
	
	if (!item)
	{
		printf("No SCM found in SCMImportWindow::UpdateCommand()\n");
		return;
	}
	
	BString command;
	scm_t scm = SCM_NONE;
	if (strcmp(B_TRANSLATE("Mercurial"), item->Label()) == 0)
	{
		scm = SCM_HG;
		//command = "hg ";
	}
	else if (strcmp(B_TRANSLATE("Git"), item->Label()) == 0)
	{
		scm = SCM_GIT;
		//command = "git ";
	}
	else if (strcmp(B_TRANSLATE("Subversion"), item->Label()) == 0)
	{
		scm = SCM_SVN;
		//command = "svn ";
	}
	else
	{
		printf("Invalid SCM in SCMImportWindow::UpdateCommand()\n");
		return;
	}
	fProvider->SetSCM(scm);
	
	// The following also provides the command, as they may need
	//   to set env variables before the command (E.g. git)
	command << fProvider->GetImportCommand(fAnonymousBox->Value() == B_CONTROL_ON);
	command << " '" << gProjectPath.GetFullPath();
	
	if (fProjectBox->Text())
		command << "/" << fProjectBox->Text();
	
	command << "' ";
	fCommandView->SetText(command.String());
}


void
SCMImportWindow::DoImport(void)
{
	SourceControl *scm = NULL;
	switch (fProvider->GetSCM())
	{
		case SCM_HG:
		{
			scm = new HgSourceControl(gProjectPath.GetRef());
			break;
		}
		case SCM_GIT:
		{
			scm = new GitSourceControl(gProjectPath.GetRef());
			break;
		}
		case SCM_SVN:
		{
			scm = new SVNSourceControl(gProjectPath.GetRef());
			break;
		}
		default:
		{
			return;
		}
	}
	
	scm->SetDebugMode(true);
	scm->SetUpdateCallback(SCMOutputCallback);
	
	SCMOutputWindow *win = new SCMOutputWindow(B_TRANSLATE("Import from online"));
	win->Show();
	
	scm->SetDebugMode(true);
	scm->SetUpdateCallback(SCMOutputCallback);
	
	DPath checkoutdir(gProjectPath.GetFullPath());
	checkoutdir << fProvider->GetProjectName();
	BDirectory dir(checkoutdir.GetFullPath());
	if (dir.InitCheck() != B_OK)
		create_directory(checkoutdir.GetFullPath(), 0777);
	
	BString command("");
	command << fProvider->GetImportCommand(fAnonymousBox->Value() == B_CONTROL_ON);
	command << " '" << gProjectPath.GetFullPath();
	
	if (fProjectBox->Text())
		command << "/" << fProjectBox->Text();
	
	command << "'";
	//command << " 2>&1";
	scm->RunCustomCommand(command.String());
	
}

