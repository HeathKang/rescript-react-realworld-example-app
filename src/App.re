open Utils;

requireCSS("./App.css");

let logo = requireAssetURI("./logo.svg");

type action =
  | UpdateUser(Types.remoteUser)
  | ChangeRoute(Types.route);

type state = {
  route: Types.route,
  user: Types.remoteUser,
};

let component = ReasonReact.reducerComponent("App");

let makeLinkClass = (current, target) =>
  "nav-link" ++ (current === target ? " active" : "");

let urlToRoute = (url: ReasonReact.Router.url) : Types.route => {
  let hash = url.hash |> Js.String.split("/");
  switch (hash) {
  | [|"", "login"|] => Login
  | [|"", "register"|] => Register
  | [|"", "settings"|] => Settings
  | [|"", "editor"|] => Editor
  | [|"", "article", slug|] => Article(slug)
  | [|"", "profile", author|] => Profile(Types.Author(author))
  | [|"", "profile", author, "favorites"|] =>
    Profile(Types.Favorited(author))
  | [|"", _|]
  | [||]
  | _ => Home
  };
};

let getUser = (_payload, {ReasonReact.send}) => {
  send(UpdateUser(RemoteData.Loading));
  Js.Promise.(
    API.user()
    |> then_(result => {
         switch (result) {
         | Js.Result.Ok(json) => Js.log2("json", json)
         | Error(error) => send(UpdateUser(RemoteData.Failure(error)))
         };
         ignore() |> resolve;
       })
    |> catch(_error => {
         send(UpdateUser(RemoteData.Failure("failed to get user data")));
         ignore() |> resolve;
       })
    |> ignore
  );
};

let make = _children => {
  ...component,
  initialState: () => {
    route: urlToRoute(ReasonReact.Router.dangerouslyGetInitialUrl()),
    user: RemoteData.NotAsked,
  },
  reducer: (action, state) =>
    switch (action) {
    | UpdateUser(user) => ReasonReact.Update({...state, user})
    | ChangeRoute(route) => ReasonReact.Update({...state, route})
    },
  subscriptions: self => [
    Sub(
      () =>
        ReasonReact.Router.watchUrl(url =>
          self.send(ChangeRoute(urlToRoute(url)))
        ),
      ReasonReact.Router.unwatchUrl,
    ),
  ],
  didMount: ({handle}) => {
    handle(getUser, ());
    ReasonReact.NoUpdate;
  },
  render: ({state}) => {
    let {route, user} = state;
    let linkCx = makeLinkClass(route);
    <div>
      <nav className="navbar navbar-light">
        <div className="container">
          <a className="navbar-brand" href="/#/"> ("conduit" |> strEl) </a>
          <ul className="nav navbar-nav pull-xs-right">
            <li className="nav-item">
              <a className=(linkCx(Home)) href="/#/"> ("Home" |> strEl) </a>
            </li>
            (
              switch (user) {
              | NotAsked
              | Loading => nullEl
              | Failure(_) => nullEl
              | Success(_) =>
                <li className="nav-item">
                  <a className=(linkCx(Editor)) href="/#/editor">
                    <i className="ion-compose" />
                    (" New Post" |> strEl)
                  </a>
                </li>
              }
            )
            (
              switch (user) {
              | NotAsked
              | Loading => nullEl
              | Failure(_) => nullEl
              | Success(_) =>
                <li className="nav-item">
                  <a className=(linkCx(Settings)) href="/#/settings">
                    <i className="ion-gear-a" />
                    (" Settings" |> strEl)
                  </a>
                </li>
              }
            )
            (
              switch (user) {
              | NotAsked
              | Loading
              | Success(_) => nullEl
              | Failure(_) =>
                <li className="nav-item">
                  <a className=(linkCx(Login)) href="/#/login">
                    ("Sign in" |> strEl)
                  </a>
                </li>
              }
            )
            (
              switch (user) {
              | NotAsked
              | Loading
              | Success(_) => nullEl
              | Failure(_) =>
                <li className="nav-item">
                  <a className=(linkCx(Register)) href="/#/register">
                    ("Sign up" |> strEl)
                  </a>
                </li>
              }
            )
          </ul>
        </div>
      </nav>
      (
        switch (route) {
        | Login => <Login />
        | Register => <Register />
        | Settings => <Settings />
        | Editor => <Editor />
        | Profile(author) => <Profile author />
        | Article(slug) => <Article slug user />
        | Home => <Home />
        }
      )
      <footer>
        <div className="container">
          <a href="/" className="logo-font"> ("conduit" |> strEl) </a>
          <span className="attribution">
            ("An interactive learning project from " |> strEl)
            <a href="https://thinkster.io"> ("Thinkster" |> strEl) </a>
            (". Code & design licensed under MIT." |> strEl)
          </span>
        </div>
      </footer>
    </div>;
  },
};

let default = ReasonReact.wrapReasonForJs(~component, _jsProps => make([||]));